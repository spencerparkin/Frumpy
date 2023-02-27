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
		Quaternion(const Vector3& vector);
		virtual ~Quaternion();

		void Identity();
		bool Invert(const Quaternion& quat);
		bool Normalize();
		void Conjugate();
		double SquareMagnitude() const;

		void Scale(double scalar);

		void Add(const Quaternion& leftQuat, const Quaternion& rightQuat);
		void Subtract(const Quaternion& leftQuat, const Quaternion& rightQuat);

		void Multiply(const Quaternion& leftQuat, const Quaternion& rightQuat);
		bool Divide(const Quaternion& leftQuat, const Quaternion& rightQuat);

		bool SetFromMatrix(const Matrix3x3& rotationMatrix);
		bool GetToMatrix(Matrix3x3& rotationMatrix) const;
		
		bool SetFromAxisAngle(const Vector3& axis, double angle);
		bool GetToAxisAngle(Vector3& axis, double& angle) const;

		void TransformVector(const Vector3& vector, Vector3& vectorTransform) const;

		bool Interpolate(const Quaternion& quatA, const Quaternion& quatB, double alpha);

		double w, x, y, z;
	};

	FRUMPY_API Quaternion operator+(const Quaternion& leftQuat, const Quaternion& rightQuat);
	FRUMPY_API Quaternion operator-(const Quaternion& leftQuat, const Quaternion& rightQuat);
	FRUMPY_API Quaternion operator*(const Quaternion& leftQuat, const Quaternion& rightQuat);
	FRUMPY_API Quaternion operator/(const Quaternion& leftQuat, const Quaternion& rightQuat);
}