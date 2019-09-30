#include "mathLib3D.h"
#include "particle3d.h"
#include <cstdlib>

Particle3D::Particle3D() {
	// generate random position inside the box
	float xPos = (rand() % 10) - 5.2;
	float yPos = (rand() % 10) - 5.2;
	float zPos = (rand() % 10) - 0.2;
	this->position = Point3D(xPos, yPos, zPos);

	// generate 3 random color floats between 0 and 1
	float red = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float green = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	float blue = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	this->color[0] = red;
	this->color[1] = green;
	this->color[2] = blue;

	// size between 10 and 20
	this->size = (rand() % 10) + 10;

	// direction particle is moving in (initially just points at origin)
	this->direction = Vec3D();

	// range of the particle between 1.0 and 5.0
	this->range = ((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 4.0) + 1.0;
	// base speed of the particle
	this->speed = 0.01;
	// friction acting on the particle
	this->friction = 0.0005;
	// overall velocity of the particle, computed each frame based on speed, friction, direction.
	this->velocity = 0;

	this->halo = false;
}