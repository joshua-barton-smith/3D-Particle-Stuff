#include <math.h>
#include "mathLib3D.h"

Point3D::Point3D() : Point3D(0.0, 0.0, 0.0) {}

Point3D::Point3D(float inX, float inY, float inZ) {
	mX = inX;
	mY = inY;
	mZ = inZ;
}

float Point3D::distanceTo(Point3D other) {
	return sqrt(pow(other.mX - mX, 2) + pow(other.mY - mY, 2) + pow(other.mZ - mZ, 2));
}

float Point3D::fastDistanceTo(Point3D other) {
	return pow(other.mX - mX, 2) + pow(other.mY - mY, 2) + pow(other.mZ - mZ, 2);
}

Vec3D::Vec3D() : Vec3D(0.0, 0.0, 0.0) {}

Vec3D::Vec3D(float inX, float inY, float inZ) {
	mX = inX;
	mY = inY;
	mZ = inZ;
}

float Vec3D::length() {
	return sqrt(pow(mX, 2) + pow(mY, 2) + pow(mZ, 2));
}

Vec3D Vec3D::normalize() {
	return Vec3D(mX/length(), mY/length(), mZ/length());
}

Vec3D Vec3D::multiply(float scalar) {
	return Vec3D(mX*scalar, mY*scalar, mZ*scalar);
}

Vec3D Vec3D::cross(Vec3D other) {
	return Vec3D((mY*other.mZ) - (mZ*other.mY), (mZ*other.mX) - (mX*other.mZ), (mX*other.mY) - (mY*other.mX));
}

Point3D Vec3D::movePoint(Point3D source) {
	return Point3D(source.mX + mX, source.mY + mY, source.mZ + mZ);
}

Vec3D Vec3D::createVector(Point3D p1, Point3D p2) {
	return Vec3D(p2.mX - p1.mX, p2.mY - p1.mY, p2.mZ - p1.mZ);
}