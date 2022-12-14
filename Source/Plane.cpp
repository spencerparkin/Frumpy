#include "Plane.h"
#include "Vector.h"
#include "Triangle.h"
#include <math.h>

using namespace Frumpy;

Plane::Plane()
{
	this->distance = 0.0;
}

Plane::Plane(const Vector& point, const Vector& normal)
{
	this->SetFromPointAndVector(point, normal);
}

/*virtual*/ Plane::~Plane()
{
}

bool Plane::SetFromPointAndVector(const Vector& point, const Vector& normal)
{
	this->unitNormal = normal;
	if (!this->unitNormal.Normalize())
		return false;

	this->distance = Vector::Dot(point, this->unitNormal);
	return true;
}

bool Plane::GetToPointAndVector(Vector& point, Vector& normal) const
{
	normal = this->unitNormal;
	point = this->unitNormal;
	point.Scale(this->distance);
	return true;
}

void Plane::SetFromTriangle(const Triangle& triangle)
{
	Vector normal;
	normal.Cross(triangle.vertex[1] - triangle.vertex[0], triangle.vertex[2] - triangle.vertex[0]);
	this->SetFromPointAndVector(triangle.vertex[0], normal);
}

double Plane::SignedDistanceToPoint(const Vector& point) const
{
	return Vector::Dot(point, this->unitNormal) - this->distance;
}

bool Plane::ContainsPoint(const Vector& point, double planeThickness) const
{
	double distance = this->SignedDistanceToPoint(point);
	return fabs(distance) <= planeThickness;
}

bool Plane::RayCast(const Vector& rayOrigin, const Vector& rayDirection, double& lambda) const
{
	double numer = this->distance - Vector::Dot(rayOrigin, this->unitNormal);
	double denom = Vector::Dot(rayDirection, this->unitNormal);
	lambda = numer / denom;
	if (lambda != lambda || lambda < 0.0)
		return false;
	return true;
}