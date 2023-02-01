#include "Plane.h"
#include "Vector.h"
#include "Triangle.h"
#include "Matrix.h"
#include <math.h>

using namespace Frumpy;

Plane::Plane()
{
	this->unitNormal.SetComponents(0.0, 0.0, 0.0);
	this->distance = 0.0;
}

Plane::Plane(const Vector& point, const Vector& normal)
{
	this->SetFromPointAndVector(point, normal);
}

/*virtual*/ Plane::~Plane()
{
}

bool Plane::IsEqualTo(const Plane& plane, double eps /*= FRUMPY_EPS*/) const
{
	if (fabs(this->distance - plane.distance) >= eps)
		return false;

	if (!this->unitNormal.IsEqualTo(plane.unitNormal, eps))
		return false;

	return true;
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

void Plane::Transform(const Matrix& transformMatrix, bool isRigidBodyTransform)
{
	Vector point, normal;
	this->GetToPointAndVector(point, normal);

	if (isRigidBodyTransform)
	{
		transformMatrix.TransformPoint(point, point);
		transformMatrix.TransformVector(normal, normal);
	}
	else
	{
		// TODO: Apply inverse transpose to normal?
	}

	this->SetFromPointAndVector(point, normal);
}