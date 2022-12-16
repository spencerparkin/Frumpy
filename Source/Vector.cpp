#include "Vector.h"
#include <math.h>

using namespace Frumpy;

Vector::Vector()
{
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

Vector::Vector(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector::Vector(const Vector& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

/*virtual*/ Vector::~Vector()
{
}

void Vector::SetComponents(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Vector::GetComponents(double& x, double& y, double& z) const
{
	x = this->x;
	y = this->y;
	z = this->z;
}

void Vector::operator=(const Vector& vector)
{
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

void Vector::Add(const Vector& leftVector, const Vector& rightVector)
{
	this->x = leftVector.x + rightVector.x;
	this->y = leftVector.y + rightVector.y;
	this->z = leftVector.z + rightVector.z;
}

void Vector::Subtract(const Vector& leftVector, const Vector& rightVector)
{
	this->x = leftVector.x - rightVector.x;
	this->y = leftVector.y - rightVector.y;
	this->z = leftVector.z - rightVector.z;
}

void Vector::Cross(const Vector& leftVector, const Vector& rightVector)
{
	this->x = leftVector.y * rightVector.z - leftVector.z * rightVector.y;
	this->y = leftVector.z * rightVector.x - leftVector.x * rightVector.z;
	this->z = leftVector.x * rightVector.y - leftVector.y * rightVector.x;
}

/*static*/ double Vector::Dot(const Vector& leftVector, const Vector& rightVector)
{
	double dot =
		leftVector.x * rightVector.x +
		leftVector.y * rightVector.y +
		leftVector.z * rightVector.z;

	return dot;
}

double Vector::Length() const
{
	return sqrt(Dot(*this, *this));
}

void Vector::Scale(double scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}

bool Vector::Normalize()
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

bool Vector::Projection(const Vector& vectorA, const Vector& vectorB)
{
	*this = vectorB;
	if (!this->Normalize())
		return false;

	double projectedLength = Dot(vectorA, *this);
	this->Scale(projectedLength);
	return true;
}

bool Vector::Rejection(const Vector& vectorA, const Vector& vectorB)
{
	Vector projection;
	if (!projection.Projection(vectorA, vectorB))
		return false;

	this->Subtract(vectorA, projection);
	return true;
}

bool Vector::Rotation(const Vector& vector, const Vector& axis, double angle)
{
	Vector unitAxis(axis);
	if (!unitAxis.Normalize())
		return false;

	Vector projection;
	if (!projection.Projection(vector, axis))
		return false;

	Vector xAxis;
	xAxis.Subtract(vector, projection);
	if (!xAxis.Normalize())
		return false;

	Vector yAxis;
	yAxis.Cross(axis, xAxis);
	if (!yAxis.Normalize())
		return false;

	double length = vector.Length();
	Vector rejection = length * (xAxis * cos(angle) + yAxis * sin(angle));

	this->Add(projection, rejection);

	return true;
}

namespace Frumpy
{
	Vector operator+(const Vector& leftVector, const Vector& rightVector)
	{
		Vector result;
		result.Add(leftVector, rightVector);
		return result;
	}

	Vector operator-(const Vector& leftVector, const Vector& rightVector)
	{
		Vector result;
		result.Subtract(leftVector, rightVector);
		return result;
	}

	Vector operator*(const Vector& vector, double scalar)
	{
		Vector result(vector);
		result.Scale(scalar);
		return result;
	}

	Vector operator*(double scalar, const Vector& vector)
	{
		Vector result(vector);
		result.Scale(scalar);
		return result;
	}
}