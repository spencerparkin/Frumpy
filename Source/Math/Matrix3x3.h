#pragma once

#include "Defines.h"

namespace Frumpy
{
	class Quaternion;
	class Vector3;

	// Methods that assume we have a rotation matrix here (orthogonal?) are left
	// undefined in the case that the matrix is not really a rotation matrix.
	class FRUMPY_API Matrix3x3
	{
	public:
		Matrix3x3();
		Matrix3x3(const Matrix3x3& matrix);
		Matrix3x3(const Quaternion& quat);
		Matrix3x3(const Vector3& axis, double angle);
		virtual ~Matrix3x3();

		void Identity();

		void GetAxes(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;
		void SetAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

		void Multiply(const Matrix3x3& leftMatrix, const Matrix3x3& rightMatrix);
		bool Divide(const Matrix3x3& leftMatrix, const Matrix3x3& rightMatrix);

		bool Invert(const Matrix3x3& matrix);
		void Transpose(const Matrix3x3& matrix);

		double Determinant() const;

		void SetRotation(double yaw, double pitch, double roll);
		void GetRotation(double& yaw, double& pitch, double& roll) const;

		void SetRotation(const Vector3& axis, double angle);
		void GetRotation(Vector3& axis, double& angle) const;

		void SetRotation(const Quaternion& quat);
		void GetRotation(Quaternion& quat) const;

		void InterpolateRotations(const Matrix3x3& matrixA, const Matrix3x3& matrixB, double alpha);

		double ele[3][3];
	};

	FRUMPY_API Matrix3x3 operator*(const Matrix3x3& leftMatrix, const Matrix3x3& rightMatrix);
}