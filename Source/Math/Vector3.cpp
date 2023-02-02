#include "Math/Vector3.h"
#include <math.h>

using namespace Frumpy;

Vector3::Vector3()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

Vector3::Vector3(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3::Vector3(const Vector3& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

/*virtual*/ Vector3::~Vector3()
{
}

void Vector3::SetComponents(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Vector3::GetComponents(double& x, double& y, double& z) const
{
	x = this->x;
	y = this->y;
	z = this->z;
}

Vector3& Vector3::operator=(const Vector3& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;

	return *this;
}

void Vector3::operator+=(const Vector3& vector)
{
	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
}

void Vector3::operator-=(const Vector3& vector)
{
	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
}

void Vector3::operator*=(double scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}

void Vector3::operator/=(double scalar)
{
	this->x /= scalar;
	this->y /= scalar;
	this->z /= scalar;
}

void Vector3::Add(const Vector3& leftVector, const Vector3& rightVector)
{
	this->x = leftVector.x + rightVector.x;
	this->y = leftVector.y + rightVector.y;
	this->z = leftVector.z + rightVector.z;
}

void Vector3::Subtract(const Vector3& leftVector, const Vector3& rightVector)
{
	this->x = leftVector.x - rightVector.x;
	this->y = leftVector.y - rightVector.y;
	this->z = leftVector.z - rightVector.z;
}

void Vector3::Cross(const Vector3& leftVector, const Vector3& rightVector)
{
	this->x = leftVector.y * rightVector.z - leftVector.z * rightVector.y;
	this->y = leftVector.z * rightVector.x - leftVector.x * rightVector.z;
	this->z = leftVector.x * rightVector.y - leftVector.y * rightVector.x;
}

bool Vector3::IsEqualTo(const Vector3& vector, double eps /*= FRUMPY_EPS*/) const
{
	return fabs(this->x - vector.x) < eps && fabs(this->y - vector.y) < eps && fabs(this->z - vector.z) < eps;
}

/*static*/ double Vector3::Dot(const Vector3& leftVector, const Vector3& rightVector)
{
	double dot =
		leftVector.x * rightVector.x +
		leftVector.y * rightVector.y +
		leftVector.z * rightVector.z;

	return dot;
}

/*static*/ double Vector3::AngleBetween(const Vector3& vectorA, const Vector3& vectorB)
{
	Vector3 unitVectorA = vectorA;
	Vector3 unitVectorB = vectorB;

	unitVectorA.Normalize();
	unitVectorB.Normalize();

	return acos(Vector3::Dot(unitVectorA, unitVectorB));
}

double Vector3::Length() const
{
	return sqrt(Dot(*this, *this));
}

void Vector3::Scale(double scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}

bool Vector3::Normalize()
{
	double length = this->Length();
	if (length == 0.0)
		return false;

	double scalar = 1.0 / length;
	if (scalar != scalar)
		return false;

	this->Scale(scalar);
	return true;
}

bool Vector3::Projection(const Vector3& vectorA, const Vector3& vectorB)
{
	*this = vectorB;
	if (!this->Normalize())
		return false;

	double projectedLength = Dot(vectorA, *this);
	this->Scale(projectedLength);
	return true;
}

bool Vector3::Rejection(const Vector3& vectorA, const Vector3& vectorB)
{
	Vector3 projection;
	if (!projection.Projection(vectorA, vectorB))
		return false;

	this->Subtract(vectorA, projection);
	return true;
}

bool Vector3::Rotation(const Vector3& vector, const Vector3& axis, double angle)
{
	Vector3 unitAxis(axis);
	if (!unitAxis.Normalize())
		return false;

	Vector3 projection = unitAxis * Vector3::Dot(vector, unitAxis);
	Vector3 rejection = vector - projection;
	double length = rejection.Length();
	if(length == 0.0)
	{
		*this = vector;
		return true;
	}

	Vector3 xAxis(rejection);
	if (!xAxis.Normalize())
		return false;

	Vector3 yAxis;
	yAxis.Cross(axis, xAxis);
	if (!yAxis.Normalize())
		return false;

	rejection = length * (xAxis * cos(angle) + yAxis * sin(angle));
	this->Add(projection, rejection);
	return true;
}

void Vector3::Lerp(const Vector3& vectorA, const Vector3& vectorB, double alpha)
{
	*this = vectorA * (1.0 - alpha) + vectorB * alpha;
}

void Vector3::Slerp(const Vector3& vectorA, const Vector3& vectorB, double alpha)
{
	//...
}

namespace Frumpy
{
	Vector3 operator+(const Vector3& leftVector, const Vector3& rightVector)
	{
		Vector3 result;
		result.Add(leftVector, rightVector);
		return result;
	}

	Vector3 operator-(const Vector3& leftVector, const Vector3& rightVector)
	{
		Vector3 result;
		result.Subtract(leftVector, rightVector);
		return result;
	}

	Vector3 operator*(const Vector3& vector, double scalar)
	{
		Vector3 result(vector);
		result.Scale(scalar);
		return result;
	}

	Vector3 operator*(double scalar, const Vector3& vector)
	{
		Vector3 result(vector);
		result.Scale(scalar);
		return result;
	}

	Vector3 operator/(const Vector3& vector, double scalar)
	{
		Vector3 result(vector);
		result.Scale(1.0 / scalar);
		return result;
	}
}