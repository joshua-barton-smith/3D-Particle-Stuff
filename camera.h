#ifndef CAMERA_H
#define CAMERA_H

#include "mathLib3D.h"
#include <math.h>

/**
* Represents a camera which looks at a 3D scene.
*/
class Camera {
public:
	// construct the camera object
	Camera(Vec3D camPos, Vec3D camTgt);

	// Represents position of the camera in 3D space
	Vec3D camPos;
	// Represents the point the camera is looking towards
	Vec3D camTgt;
	// Represents direction from camPos to camTgt
	Vec3D camDir;

	// Vector pointing directly upwards
	Vec3D up;
	// Vector pointing to the right from the camera's perspective
	Vec3D camRight;
	// Vector pointing up from the camera's perspective
	Vec3D camUp;

	// Vector representing the front of the camera (gets rotated as needed)
	Vec3D camFront;

	// Angles of rotation for the camera.
	float pitch;
	float yaw;

	// camera movement/rotation speed.
	float camSpeed;
	float rotSpeed;

	// Sets up perspective view
	void setupPerspective();

	// Looks at the point specified by camFront/camPos.
	void lookAt();

	// updates the rotation based on computed x/y offsets
	void updateRotation(float xoff, float yoff);

	// applies rotations to camFront based on the pitch/yaw.
	void applyRotation();

	// applies movements to camPos based on the input movement array.
	void applyMovement(bool movement[]);
};

#endif