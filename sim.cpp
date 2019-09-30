#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/freeglut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <sstream>
#include "mathLib3D.h"
#include "particle3d.h"
#include "camera.h"

// Size of the screen, gets adjusted by the reshape func
int screensize[] = {600, 600};

// keyboard inputs - w,s,a,d
bool keys_down[] = {false, false, false, false};
// mouse input state
bool mouse_buttons[2] = {false, false};

// Camera object
Camera camera = Camera(Vec3D(0.0, 0.0, 7.0), Vec3D(0.0, 0.0, 0.0));

// list of all particles
std::vector<Particle3D> particles;

// these variables are used for messages displayed on screen for short durations
// this is the number of frames to display a message for
int renderFrames = 0;
// average range of all particles
float avg_range = 0;
// average speed
float avg_speed = 0;

// if the animation is paused
bool paused = false;

// central mouse positions
float centerX = 300, centerY = 300;

// maximums for various particle properties
const float MAX_RANGE = 6.0;
const float MIN_RANGE = 0.3;
const float MAX_SPEED = 0.015;
const float MIN_SPEED = 0.006;

// show the instructions?
bool show_instructions = true;

// full instructions to be displayed
const char *text_instructions = "Welcome to the Particle Animation!\n"
"This doesn't run well on gpu1, try it locally.\n"
"It will show particles on the screen which you can interact with.\n"
"You can move around using WSAD and the mouse.\n"
"The left and right mouse buttons will attract and\ndeflect particles respectively.\n"
"Particles can be added and deleted by hitting 'N' or 'M'.\n"
"The range for a particle to be affected by the mouse\ncan be adjusted with + or -.\n"
"The speed of the particles can be increased or decreased\nby pressing up or down arrows.\n"
"The animation can be paused at any time with the space bar.\n"
"More particles can be added in bulk with 'G',\n"
"Or you can hit 'R' to erase all particles and start fresh.\n"
"You can quit at any time by hitting 'Q' or Escape.\n\n"
"Now click to begin!";

/**
* Generates a new set of particles, spawned at the center and with random velocities.
*/
void genParticles(bool clear, int minCount, int maxCount) {
  // empty out the list
  if (clear) particles.clear();
  // random number of particles
  int particleCount = (rand() % maxCount) + minCount;
  // empty the avg range
  avg_range = 0;
  avg_speed = 0;
  for (int i = 0; i < particleCount; i++) {
    // construct new particle
    Particle3D p = Particle3D();
    // spawn the particle in the center
    p.position = Point3D(0, 0, 5);
    // randomize direction and velo
    // assign a random direction to the particle
    int randX = (rand() % 10) - 5;
    int randY = (rand() % 10) - 5;
    int randZ = (rand() % 10);
    p.direction = Vec3D::createVector(p.position, Point3D(randX, randY, randZ)).normalize();
    // assign a random velocity to the particle
    float randVelo = 2 * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    p.velocity = randVelo;

    // add to list
    particles.push_back(p);

    avg_range += p.range;
    avg_speed += p.speed;
  }
  avg_range /= particleCount;
  avg_speed /= particleCount;
}

/**
* This function computes motion for all particles on the screen.
* How this will work:
* 1. initially all particles at rest (p->velocity = 0).
* 2. particle starts to move if mouse clicked while camera in range of it (assume lmb)
* 3. p->direction is stored with direction of the camera from the particle
* 4. p->velocity is increased by p->speed and decreased by p->size*p->friction
* 5. with each loop p->direction is continually updated (so particle will not overshoot)
* 6. when lmb released, we no longer increase p->velocity by p->speed, only decrease by p->friction
*/
void computeParticleMotion() {
  // direction the camera is looking
  Point3D cp = Point3D(camera.camPos.mX + camera.camFront.mX, camera.camPos.mY + camera.camFront.mY, camera.camPos.mZ + camera.camFront.mZ);
  for (int i = 0; i < particles.size(); i++) {
    // get the particle as a pointer because I don't like working with long particles[i]
    Particle3D* p = &particles[i];
    // check if this particle is in range of the camera, and lmb is down
    if (p->position.distanceTo(cp) <= p->range && mouse_buttons[0]) {
      // compute the direction the particle goes in to follow the camera
      p->direction = Vec3D::createVector(p->position, cp).normalize();
      // increase the velocity of the particle
      p->velocity += p->speed;

      p->halo = true;
    }
    // check if this particle is in range of the camera, and rmb is down
    else if (p->position.distanceTo(cp) <= p->range && mouse_buttons[1]) {
      // compute the direction the particle goes in to go away from the camera
      p->direction = Vec3D::createVector(cp, p->position).normalize();
      // increase the velocity of the particle
      p->velocity += p->speed;

      // display an indicator on affected particles
      p->halo = true;
    } else p->halo = false; // remove indicator from unaffected particles
    // decrease the velocity of the particle based on friction, to a minimum of 0
    p->velocity -= ((float)(p->size) * p->friction);
    if (p->velocity < 0) p->velocity = 0;
  }
}

/**
* Applies motion to all particles based on their velocity and direction
*/
void moveParticles() {
  for (int i = 0; i < particles.size(); i++) {
    // get the particle as a pointer because I don't like working with long particles[i]
    Particle3D* p = &particles[i];
    // we need to check if the particle has hit an invalid position (i.e. if it's passed through a wall)
    float pX = p->position.mX;
    float pY = p->position.mY;
    float pZ = p->position.mZ;
    // if it has passed through a wall, bounce off the wall.
    // this is really easy to do by reversing direction on 1 axis
    // (thank you to https://stackoverflow.com/questions/573084/how-to-calculate-bounce-angle)
    if (pX < -4.9 || pX > 4.9) p->direction.mX *= -1; 
    if (pY < -4.9 || pY > 4.9) p->direction.mY *= -1; 
    if (pZ < 0.1 || pZ > 9.9) p->direction.mZ *= -1; 
    // move the particle based on direction + velocity
    p->position = p->direction.multiply(p->velocity).movePoint(p->position);
  }
}

/**
* Renders a single particle based on its properties
*/
void drawParticle(Particle3D p) {
  // color the point
  glColor3f(p.color[0], p.color[1], p.color[2]);
  // size the point
  glPointSize(p.size);

  // render the point
  glBegin(GL_POINTS);
    glVertex3f(p.position.mX, p.position.mY, p.position.mZ);
  glEnd();

  // if the particle is being affected by the mouse, render a halo surrounding it.
  if (p.halo) {
    glColor4f(1.0, 0.0, 0.0, 0.3);
    glPointSize(p.size+5);
    glBegin(GL_POINTS);
      glVertex3f(p.position.mX, p.position.mY, p.position.mZ);
    glEnd();
  }
}

/**
* Main rendering of the particle simulation.
*/
void particleSim() {
    // iterate over all particles to be rendered
    for (int i = 0; i < particles.size(); i++) {
      drawParticle(particles[i]);
    }
}

/**
* Draws the 6 walls which will contain all particles.
*/
void drawWalls() {
  // front wall
  glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
      glVertex3f(-5.0, -5.0, 0.0);
      glVertex3f(-5.0, 5.0, 0.0);
      glVertex3f(5.0, 5.0, 0.0);
      glVertex3f(5.0, -5.0, 0.0);
    glEnd();
  glPopMatrix();

  // back wall
  glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_QUADS);
      glVertex3f(-5.0, -5.0, 10.0);
      glVertex3f(-5.0, 5.0, 10.0);
      glVertex3f(5.0, 5.0, 10.0);
      glVertex3f(5.0, -5.0, 10.0);
    glEnd();
  glPopMatrix();

  // left wall
  glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
      glVertex3f(-5.0, -5.0, 0.0);
      glVertex3f(-5.0, -5.0, 10.0);
      glVertex3f(-5.0, 5.0, 10.0);
      glVertex3f(-5.0, 5.0, 0.0);
    glEnd();
  glPopMatrix();

  // right wall
  glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
      glVertex3f(5.0, -5.0, 0.0);
      glVertex3f(5.0, -5.0, 10.0);
      glVertex3f(5.0, 5.0, 10.0);
      glVertex3f(5.0, 5.0, 0.0);
    glEnd();
  glPopMatrix();

  // top wall
  glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
      glVertex3f(-5.0, 5.0, 10.0);
      glVertex3f(5.0, 5.0, 10.0);
      glVertex3f(5.0, 5.0, 0.0);
      glVertex3f(-5.0, 5.0, 0.0);
    glEnd();
  glPopMatrix();

  // bottom wall
  glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
      glVertex3f(-5.0, -5.0, 10.0);
      glVertex3f(5.0, -5.0, 10.0);
      glVertex3f(5.0, -5.0, 0.0);
      glVertex3f(-5.0, -5.0, 0.0);
    glEnd();
  glPopMatrix();
}

/**
* Operating instructions rendered to the screen.
*/
void instructions() {
  glColor3f(1, 1, 1);

  glRasterPos2f(-1, 0.7);
  glutBitmapString(GLUT_BITMAP_HELVETICA_18, reinterpret_cast<const unsigned char *>(text_instructions));
}

/**
* Contains rendering steps for shapes and particles.
*/
void shapeRender() {
  // draw the box and all particles.
  drawWalls();
  particleSim();
}

/**
* Handles all camera movements and rotations.
*/
void cameraMovement() {
  camera.applyMovement(keys_down);

  // enforce boundaries for camera positioning based on the walls
  // this is a terrible way to implement this but it doesn't matter,
  // the walls are fixed location and the only solid objects in the scene.
  if (camera.camPos.mX > 4.7) camera.camPos.mX = 4.7;
  if (camera.camPos.mY > 4.7) camera.camPos.mY = 4.7;
  if (camera.camPos.mZ > 9.7) camera.camPos.mZ = 9.7;
  if (camera.camPos.mX < -4.7) camera.camPos.mX = -4.7;
  if (camera.camPos.mY < -4.7) camera.camPos.mY = -4.7;
  if (camera.camPos.mZ < 0.3) camera.camPos.mZ = 0.3;

  camera.applyRotation();
}

void messageRender() {
  // this will show a message iff variables have been changed, to provide info to user
  if (renderFrames > 0) {
    // direction of the message to be rendered
    Point3D cp = Point3D(camera.camPos.mX + camera.camFront.mX, camera.camPos.mY + camera.camFront.mY, camera.camPos.mZ + camera.camFront.mZ);
    std::stringstream stream;
    stream << "Average particle range: " << avg_range << "\nAverage particle speed: " << avg_speed << "\nParticle count: " << particles.size();
    std::string output = stream.str();
    if (paused) output = "Animation Paused\n\n" + output;

    glColor4f(1, 0, 0, 0.8);

    glRasterPos3f(cp.mX, cp.mY, cp.mZ);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, reinterpret_cast<const unsigned char *>
      (output.c_str()));

    renderFrames--;
  }
}

/************************************
* Bunch of glut callbacks below here
*************************************/

/**
* Display callback, just renders stuff and swaps buffers
*/
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (show_instructions) {
    instructions();
  } else { // setup camera and whatever shapes (walls, particles, ...)
    camera.setupPerspective();
    camera.lookAt();

    shapeRender();
    messageRender();
  }

  glutSwapBuffers();
}

/**
* Handles regular keyboard inputs (e.g. w/s/a/d for movement)
*/
void handleKeyboard(unsigned char key, int _x, int _y) {
  if (!paused) {
    switch(key) {
      case 'w':
      {
        keys_down[0] = true;
        break;
      }
      case 's':
      {
        keys_down[1] = true;
        break;
      }
      case 'a':
      {
        keys_down[2] = true;
        break;
      }
      case 'd':
      {
        keys_down[3] = true;
        break;
      }
      case 'r':
      {
        // r key regenerates particles from scratch
        genParticles(true, 2000, 3000);
        break;
      }
      case 'g':
      {
        // g key generates some new particles
        genParticles(false, 20, 50);
        break;
      }
      case 'n':
      {
        {// create a new particle at the camera position.
          Point3D cp = Point3D(camera.camPos.mX + camera.camFront.mX, camera.camPos.mY + camera.camFront.mY, camera.camPos.mZ + camera.camFront.mZ);
          Particle3D p = Particle3D();
          p.position = cp;
          particles.push_back(p);
        }
      }
      case 'm':
      {
        {// delete the particle closest to the camera position
          Point3D cp = Point3D(camera.camPos.mX + camera.camFront.mX, camera.camPos.mY + camera.camFront.mY, camera.camPos.mZ + camera.camFront.mZ);
          int closest = 0;
          float closestDist = 100000;
          for (int i = 0; i < particles.size(); i++) {
            float fdt = cp.fastDistanceTo(particles[i].position);
            if(fdt < closestDist) {
              closestDist = fdt;
              closest = i;
            }
          }
          particles.erase(particles.begin() + closest);
        }
      }
      case '+':
      {
        // will show the user the change to overall average range
        renderFrames = 60;
        avg_range = 0;
        // increase range for all particles to a max of 5.0
        for (int i = 0; i < particles.size(); i++) {
          particles[i].range += 0.13;
          if (particles[i].range > MAX_RANGE) particles[i].range = MAX_RANGE;
          avg_range += particles[i].range;
        }
        // take the average
        avg_range /= particles.size();
        break;
      }
      case '-':
      {
        // will show the user the change to overall average range
        renderFrames = 60;
        avg_range = 0;
        // reduce range by a fixed amount for all particles
        for (int i = 0; i < particles.size(); i++) {
          particles[i].range -= 0.13;
          if (particles[i].range < MIN_RANGE) particles[i].range = MIN_RANGE;
          avg_range += particles[i].range;
        }
        // take the average
        avg_range /= particles.size();
        break;
      }
    }
  }
  // quit
  if (key == 'q' || key == 27) {
    exit(0);
  } else if (key == ' ') {
    paused = !paused;
  }
}

/**
* Handle arrow key inputs.
*/
void special(int key, int x, int y) {
  if (!show_instructions && !paused) {
    switch(key) {
      case GLUT_KEY_UP:
      {
        // show message for 60 frames
        renderFrames = 60;
        avg_speed = 0;
        for (int i = 0; i < particles.size(); i++) {
          particles[i].speed += 0.002;
          if (particles[i].speed > MAX_SPEED) particles[i].speed = MAX_SPEED;
          avg_speed += particles[i].speed;
        }
        avg_speed /= particles.size();
        break;
      }
      case GLUT_KEY_DOWN:
      {
        // show message for 60 frames
        renderFrames = 60;
        avg_speed = 0;
        for (int i = 0; i < particles.size(); i++) {
          particles[i].speed -= 0.002;
          if (particles[i].speed < MIN_SPEED) particles[i].speed = MIN_SPEED;
          avg_speed += particles[i].speed;
        }
        avg_speed /= particles.size();
        break;
      }
    }
  }
}

/**
* Handles keys being released, used bc w/s/a/d for movement are toggled ON while held.
*/
void handleKeyboardUp(unsigned char key, int _x, int _y) {
  switch(key) {
    case 'w':
    {
      keys_down[0] = false;
      break;
    }
    case 's':
    {
      keys_down[1] = false;
      break;
    }
    case 'a':
    {
      keys_down[2] = false;
      break;
    }
    case 'd':
    {
      keys_down[3] = false;
      break;
    }
  }
}

/**
* Mouse click function.
*/
void mouse(int button, int state, int x, int y) {
  if (show_instructions) show_instructions = false;
  // remove instructions if left button is clicked while they're on screen
  else if (!paused) {
    // check whether the button is up or down
    if (state == GLUT_DOWN) {
      // set appropriate button state to true
      if (button == GLUT_LEFT_BUTTON) mouse_buttons[0] = true;
      else if (button == GLUT_RIGHT_BUTTON) mouse_buttons[1] = true;
    } else {
      // set appropriate button state to false
      if (button == GLUT_LEFT_BUTTON) mouse_buttons[0] = false;
      else if (button == GLUT_RIGHT_BUTTON) mouse_buttons[1] = false;
    }
  }
}

/**
* Keeps track of mouse motion and updates pitch/yaw accordingly.
*/
void mouseMotion(int x, int y) {
  float xoff = x - centerX;
  float yoff = y - centerY;

  camera.updateRotation(xoff, yoff);

  glutWarpPointer(centerX, centerY);
}

void FPS(int val) {
  if (!paused) {
    cameraMovement();
    computeParticleMotion();
    moveParticles();
  }
  glutPostRedisplay();
  glutTimerFunc(17, FPS, val);
}

/* main function - program entry point */
int main(int argc, char** argv)
{
  // seed random number generator
  srand(time(NULL));

  // come up with a random particle count (2000 - 3000)
  genParticles(true, 2000, 3000);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE);
  glutInitWindowSize(600,600);
  glutInitWindowPosition(300,0);
  glutCreateWindow("Assignment 2 - Particle Sim - 3D");

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClearDepth(1.0);
  glShadeModel(GL_FLAT);

  glutSetCursor(GLUT_CURSOR_NONE);

  // enable blending for alpha vals
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glutKeyboardFunc(handleKeyboard);
  glutKeyboardUpFunc(handleKeyboardUp);
  glutSpecialFunc(special);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutPassiveMotionFunc(mouseMotion); 
  glutDisplayFunc(display);
  glutTimerFunc(17, FPS, 0);

  glutMainLoop();

  return 0;
}
