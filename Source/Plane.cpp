#include "Plane.h"
#include "Vector3.h"
#include "Triangle.h"
#include "Matrix4x4.h"
#include <math.h>

using namespace Frumpy;

Plane::Plane()
{
	this->unitNormal.SetComponents(0.0, 0.0, 0.0);
	this->distance = 0.0;
}

Plane::Plane(const Vector3& point, const Vector3& normal)
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

bool Plane::SetFromPointAndVector(const Vector3& point, const Vector3& normal)
{
	this->unitNormal = normal;
	if (!this->unitNormal.Normalize())
		return false;

	this->distance = Vector3::Dot(point, this->unitNormal);
	return true;
}

bool Plane::GetToPointAndVector(Vector3& point, Vector3& normal) const
{
	normal = this->unitNormal;
	point = this->unitNormal;
	point.Scale(this->distance);
	return true;
}

void Plane::SetFromTriangle(const Triangle& triangle)
{
	Vector3 normal;
	normal.Cross(triangle.vertex[1] - triangle.vertex[0], triangle.vertex[2] - triangle.vertex[0]);
	this->SetFromPointAndVector(triangle.vertex[0], normal);
}

double Plane::SignedDistanceToPoint(const Vector3& point) const
{
	return Vector3::Dot(point, this->unitNormal) - this->distance;
}

bool Plane::ContainsPoint(const Vector3& point, double planeThickness) const
{
	double distance = this->SignedDistanceToPoint(point);
	return fabs(distance) <= planeThickness;
}

bool Plane::RayCast(const Vector3& rayOrigin, const Vector3& rayDirection, double& lambda) const
{
	double numer = this->distance - Vector3::Dot(rayOrigin, this->unitNormal);
	double denom = Vector3::Dot(rayDirection, this->unitNormal);
	lambda = numer / denom;
	if (lambda != lambda || lambda < 0.0)
		return false;
	return true;
}

void Plane::Transform(const Matrix4x4& transformMatrix, bool isRigidBodyTransform)
{
	Vector3 point, normal;
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