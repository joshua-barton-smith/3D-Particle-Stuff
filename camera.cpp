#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/freeglut.h>
#endif

#include "camera.h"
#include "mathLib3D.h"

// a significant amount of 3d camera code was converted from code in
// https://learnopengl.com/Getting-started/Camera
Camera::Camera(Vec3D camPos, Vec3D camTgt) {
	// position of camera
	this->camPos = camPos;
	// point camera is looking towards
	this->camTgt = camTgt;
	// direction camera looks in
	this->camDir = Vec3D(camPos.mX - camTgt.mX, camPos.mY - camTgt.mY, camPos.mZ - camTgt.mZ).normalize();

	// up direction (absolute)
	this->up = Vec3D(0.0, 1.0, 0.0);
	// right vector from the camera
	this->camRight = up.cross(camDir).normalize();
	// up vector from the camera
	this->camUp = camDir.cross(camRight);

	// front of the camera (where it is pointing to/looking)
	this->camFront = Vec3D(0.0, 0.0, -1.0);

	// angles of rotation
	this->pitch = 0.0;
	this->yaw = 0.0;

	// movement speed of the camera
	this->camSpeed = 0.1;
	// rotation speed of the camera
	this->rotSpeed = 0.7;
}

void Camera::setupPerspective() {
	// load projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// set up perspective with 90 fov
	gluPerspective(90, 1.0, 0.1, 100);
}

void Camera::lookAt() {
	// load modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// look at the point
	gluLookAt (this->camPos.mX, this->camPos.mY, this->camPos.mZ,
		(this->camPos.mX + this->camFront.mX), (this->camPos.mY + this->camFront.mY), (this->camPos.mZ + this->camFront.mZ),
		this->up.mX, this->up.mY, this->up.mZ);
}

void Camera::updateRotation(float xoff, float yoff) {
	// apply sensitivity to the motion
	float sensitivity = 0.03f;
	xoff *= sensitivity;
	yoff *= sensitivity;

	// adjust rotations
	this->yaw += xoff;
	this->pitch -= yoff;
}

void Camera::applyRotation() {
	// pitch is constrained because pitch gets weird outside of (-90, 90)
	// (stuff flips upside down)
	if (pitch > 89.0) pitch = 89.0;
	if (pitch < -89.0) pitch = -89.0;

	// compute the new camFront based on the pitch/yaw angles.
	float mX = cos((M_PI*this->pitch)/180) * cos((M_PI*this->yaw)/180);
	float mY = sin((M_PI*this->pitch)/180);
	float mZ = cos((M_PI*this->pitch)/180) * sin((M_PI*this->yaw)/180);

	this->camFront = Vec3D(mX, mY, mZ).normalize();
}

void Camera::applyMovement(bool movement[]) {
	// movement[0,1,2,3] = forward, back, left, right respectively.
	// each of these corresponds to particular movement on camPos, the vector representing
	// the camera's position.
	// these are applied relative to camFront, because camFront stores the direction the camera is pointing in.
	if (movement[0]) {
		Vec3D tmp = this->camFront.multiply(this->camSpeed);
		this->camPos.mX += tmp.mX;
		this->camPos.mY += tmp.mY;
		this->camPos.mZ += tmp.mZ;
	}

	if (movement[1]) {
		Vec3D tmp = this->camFront.multiply(this->camSpeed);
		this->camPos.mX -= tmp.mX;
		this->camPos.mY -= tmp.mY;
		this->camPos.mZ -= tmp.mZ;
	}

	if (movement[2]) {
		Vec3D tmp = this->camFront.cross(this->camUp).normalize().multiply(this->camSpeed);
		this->camPos.mX -= tmp.mX;
		this->camPos.mY -= tmp.mY;
		this->camPos.mZ -= tmp.mZ;
	}

	if (movement[3]) {
		Vec3D tmp = this->camFront.cross(this->camUp).normalize().multiply(this->camSpeed);
		this->camPos.mX += tmp.mX;
		this->camPos.mY += tmp.mY;
		this->camPos.mZ += tmp.mZ;
	}
}