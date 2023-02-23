#pragma once

#include "Defines.h"

namespace Frumpy
{
	class Matrix3x3;
	class Vector3;

	// In geometric algebra, these are called rotors, but here we're just going to
	// implement the more tranditional version called quaternions.
	class FRUMPY_API Quaternion
	{
	public:
		Quaternion();
		Quaternion(const Quaternion& quat);
		Quaternion(const Matrix3x3& rotationMatrix);
		Quaternion(const Vector3& unitAxis, double angle);
		virtual ~Quaternion();

		void Identity();
		bool Invert(const Quaternion& quat);
		void Normalize();
		void Conjugate();		// The inverse of a normalized quaternion is its conjugate.

		void Multiply(const Quaternion& leftQuat, const Quaternion& rightQuat);
		bool Divide(const Quaternion& leftQuat, const Quaternion& rightQuat);

		void SetFromMatrix(const Matrix3x3& rotationMatrix);
		void GetToMatrix(Matrix3x3& rotationMatrix) const;
		
		void SetFromAxisAngle(const Vector3& unitAxis, double angle);
		void GetToAxisAngle(Vector3& unitAxis, double& angle) const;

		void TransformVector(const Vector3& vector, Vector3& vectorTransform) const;

		double w, x, y, z;
	};

	FRUMPY_API Quaternion operator*(const Quaternion& leftQuat, const Quaternion& rightQuat);
}