#ifndef PARTICLE_H
#define PARTICLE_H

#include "mathLib3D.h"

class Particle3D {
public:
	Particle3D();

	Point3D position;
	float color[3];
	int size;
	Vec3D direction;
	float range;
	float speed;
	float velocity;
	float friction;

	bool halo;
};

#endif