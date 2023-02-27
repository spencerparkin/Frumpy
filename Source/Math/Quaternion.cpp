#include "Quaternion.h"
#include "Matrix3x3.h"
#include "Vector3.h"
#include <math.h>

using namespace Frumpy;

Quaternion::Quaternion()
{
	this->Identity();
}

Quaternion::Quaternion(const Quaternion& quat)
{
	this->w = quat.w;
	this->x = quat.x;
	this->y = quat.y;
	this->z = quat.z;
}

Quaternion::Quaternion(const Matrix3x3& rotationMatrix)
{
	this->SetFromMatrix(rotationMatrix);
}

Quaternion::Quaternion(const Vector3& unitAxis, double angle)
{
	this->SetFromAxisAngle(unitAxis, angle);
}

Quaternion::Quaternion(const Vector3& vector)
{
	this->w = 0.0;
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
}

/*virtual*/ Quaternion::~Quaternion()
{
}

void Quaternion::Identity()
{
	this->w = 1.0;
	this->x = 0.0;
	this->y = 0.0;
	this->z = 0.0;
}

bool Quaternion::Invert(const Quaternion& quat)
{
	double squareMag = quat.SquareMagnitude();
	if (squareMag == 0.0)
		return false;

	this->w = quat.w / squareMag;
	this->x = -quat.x / squareMag;
	this->y = -quat.y / squareMag;
	this->z = -quat.z / squareMag;

	return true;
}

bool Quaternion::Normalize()
{
	double squareMag = this->SquareMagnitude();
	if (squareMag == 0.0)
		return false;

	double mag = ::sqrt(squareMag);

	this->w /= mag;
	this->x /= mag;
	this->y /= mag;
	this->z /= mag;

	return true;
}

void Quaternion::Conjugate()
{
	this->x = -this->x;
	this->y = -this->y;
	this->z = -this->z;
}

double Quaternion::SquareMagnitude() const
{
	double squareMag =
		this->w * this->w +
		this->x * this->x +
		this->y * this->y +
		this->z * this->z;

	return squareMag;
}

void Quaternion::Scale(double scalar)
{
	this->w *= scalar;
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}

void Quaternion::Add(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	this->w = leftQuat.w + rightQuat.w;
	this->x = leftQuat.x + rightQuat.x;
	this->y = leftQuat.y + rightQuat.y;
	this->z = leftQuat.z + rightQuat.z;
}

void Quaternion::Subtract(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	this->w = leftQuat.w - rightQuat.w;
	this->x = leftQuat.x - rightQuat.x;
	this->y = leftQuat.y - rightQuat.y;
	this->z = leftQuat.z - rightQuat.z;
}

void Quaternion::Multiply(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	double a1 = leftQuat.w;
	double b1 = leftQuat.x;
	double c1 = leftQuat.y;
	double d1 = leftQuat.z;

	double a2 = rightQuat.w;
	double b2 = rightQuat.x;
	double c2 = rightQuat.y;
	double d2 = rightQuat.z;

	this->w = a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2;
	this->x = a1 * b2 + b1 * a2 + c1 * d2 - d1 * c2;
	this->y = a1 * c2 - b1 * d2 + c1 * a2 + d1 * b2;
	this->z = a1 * d2 + b1 * c2 - c1 * b2 + d1 * a2;
}

bool Quaternion::Divide(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	Quaternion rightQuatInv;
	if (!rightQuatInv.Invert(rightQuat))
		return false;

	this->Multiply(leftQuat, rightQuatInv);
	return true;
}

bool Quaternion::SetFromMatrix(const Matrix3x3& rotationMatrix)
{
	// This is Cayley's method taken from "A Survey on the Computation of Quaternions from Rotation Matrices" by Sarabandi & Thomas.

	double r11 = rotationMatrix.ele[0][0];
	double r21 = rotationMatrix.ele[1][0];
	double r31 = rotationMatrix.ele[2][0];

	double r12 = rotationMatrix.ele[0][1];
	double r22 = rotationMatrix.ele[1][1];
	double r32 = rotationMatrix.ele[2][1];

	double r13 = rotationMatrix.ele[0][2];
	double r23 = rotationMatrix.ele[1][2];
	double r33 = rotationMatrix.ele[2][2];

	this->w = 0.25 * ::sqrt(FRUMPY_SQUARED(r11 + r22 + r33 + 1.0) + FRUMPY_SQUARED(r32 - r23) + FRUMPY_SQUARED(r13 - r31) + FRUMPY_SQUARED(r21 - r12));
	this->x = 0.25 * ::sqrt(FRUMPY_SQUARED(r32 - r23) + FRUMPY_SQUARED(r11 - r22 - r33 + 1.0) + FRUMPY_SQUARED(r21 + r12) + FRUMPY_SQUARED(r31 + r13)) * FRUMPY_SIGN(r32 - r23);
	this->w = 0.25 * ::sqrt(FRUMPY_SQUARED(r13 - r31) + FRUMPY_SQUARED(r21 + r12) + FRUMPY_SQUARED(r22 - r11 - r33 + 1.0) + FRUMPY_SQUARED(r32 + r23)) * FRUMPY_SIGN(r13 - r31);
	this->z = 0.25 * ::sqrt(FRUMPY_SQUARED(r21 - r12) + FRUMPY_SQUARED(r31 + r13) + FRUMPY_SQUARED(r32 + r23) + FRUMPY_SQUARED(r33 - r11 - r22 + 1.0)) * FRUMPY_SIGN(r21 - r12);

	return true;
}

bool Quaternion::GetToMatrix(Matrix3x3& rotationMatrix) const
{
	Vector3 xAxis(1.0, 0.0, 0.0);
	Vector3 yAxis(0.0, 1.0, 0.0);
	Vector3 zAxis(0.0, 0.0, 1.0);

	Vector3 col0, col1, col2;

	this->TransformVector(xAxis, col0);
	this->TransformVector(yAxis, col1);
	this->TransformVector(zAxis, col2);

	rotationMatrix.SetAxes(col0, col1, col2);

	return true;
}

bool Quaternion::SetFromAxisAngle(const Vector3& axis, double angle)
{
	Vector3 unitAxis(axis);
	if (!unitAxis.Normalize())
		return false;

	double halfAngle = angle / 2.0;
	double sinHalfAngle = ::sin(halfAngle);
	double cosHalfAngle = ::cos(halfAngle);

	this->w = cosHalfAngle;
	this->x = axis.x * sinHalfAngle;
	this->y = axis.y * sinHalfAngle;
	this->z = axis.z * sinHalfAngle;

	return true;
}

bool Quaternion::GetToAxisAngle(Vector3& axis, double& angle) const
{
	axis.x = this->x;
	axis.y = this->y;
	axis.z = this->z;

	double sinHalfAngle = axis.Length();
	if (sinHalfAngle == 0.0)
	{
		axis.x = 1.0;
		axis.y = 0.0;
		axis.z = 0.0;
		angle = 0.0;
		return true;
	}

	double halfAngle = ::asin(sinHalfAngle);
	angle = 2.0 * halfAngle;
	axis /= sinHalfAngle;

	return true;
}

void Quaternion::TransformVector(const Vector3& vector, Vector3& vectorTransform) const
{
	Quaternion vectorQuat(vector);

	Quaternion quatInv;
	quatInv.Invert(*this);

	Quaternion vectorQuatTransformed = *this * vectorQuat * quatInv;

	vectorTransform.x = vectorQuatTransformed.x;
	vectorTransform.y = vectorQuatTransformed.y;
	vectorTransform.z = vectorQuatTransformed.z;
}

// TODO: This is probably dumb.  Fix it.
bool Quaternion::Interpolate(const Quaternion& quatA, const Quaternion& quatB, double alpha)
{
	Vector3 axisA, axisB;
	double angleA, angleB;

	if (!quatA.GetToAxisAngle(axisA, angleA))
		return false;

	if (!quatB.GetToAxisAngle(axisB, angleB))
		return false;

	Vector3 interpolatedAxis;
	interpolatedAxis.Slerp(axisA, axisB, alpha);

	double interpolatedAngle = angleA * (1.0 - alpha) + angleB * alpha;

	return this->SetFromAxisAngle(interpolatedAxis, interpolatedAngle);
}

namespace Frumpy
{
	Quaternion operator+(const Quaternion& leftQuat, const Quaternion& rightQuat)
	{
		Quaternion sum;
		sum.Add(leftQuat, rightQuat);
		return sum;
	}

	Quaternion operator-(const Quaternion& leftQuat, const Quaternion& rightQuat)
	{
		Quaternion difference;
		difference.Subtract(leftQuat, rightQuat);
		return difference;
	}

	Quaternion operator*(const Quaternion& leftQuat, const Quaternion& rightQuat)
	{
		Quaternion product;
		product.Multiply(leftQuat, rightQuat);
		return product;
	}

	Quaternion operator/(const Quaternion& leftQuat, const Quaternion& rightQuat)
	{
		Quaternion quotient;
		quotient.Multiply(leftQuat, rightQuat);
		return quotient;
	}
}