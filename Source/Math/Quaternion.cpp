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
	//...
	return true;
}

void Quaternion::Normalize()
{
	//...
}

void Quaternion::Conjugate()
{
	this->x = -this->x;
	this->y = -this->y;
	this->z = -this->z;
}

void Quaternion::Multiply(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	//...
}

bool Quaternion::Divide(const Quaternion& leftQuat, const Quaternion& rightQuat)
{
	Quaternion rightQuatInv;
	if (!rightQuatInv.Invert(rightQuat))
		return false;

	this->Multiply(leftQuat, rightQuatInv);
	return true;
}

void Quaternion::SetFromMatrix(const Matrix3x3& rotationMatrix)
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
}

void Quaternion::GetToMatrix(Matrix3x3& rotationMatrix) const
{
	Vector3 xAxis(1.0, 0.0, 0.0);
	Vector3 yAxis(0.0, 1.0, 0.0);
	Vector3 zAxis(0.0, 0.0, 1.0);

	Vector3 col0, col1, col2;

	this->TransformVector(xAxis, col0);
	this->TransformVector(yAxis, col1);
	this->TransformVector(zAxis, col2);

	rotationMatrix.SetAxes(col0, col1, col2);
}

void Quaternion::SetFromAxisAngle(const Vector3& unitAxis, double angle)
{
	//...
}

void Quaternion::GetToAxisAngle(Vector3& unitAxis, double& angle) const
{
	//...
}

void Quaternion::TransformVector(const Vector3& vector, Vector3& vectorTransform) const
{
	//...
}

namespace Frumpy
{
	Quaternion operator*(const Quaternion& leftQuat, const Quaternion& rightQuat)
	{
		Quaternion product;
		product.Multiply(leftQuat, rightQuat);
		return product;
	}
}