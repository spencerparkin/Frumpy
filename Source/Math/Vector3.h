#pragma once

#include "Defines.h"

namespace Frumpy
{
	// These are 3-dimensional vectors geared toward 3D computer graphics applications.
	class FRUMPY_API Vector3
	{
	public:
		Vector3();
		Vector3(double x, double y, double z);
		Vector3(const Vector3& vector);
		virtual ~Vector3();

		Vector3& operator=(const Vector3& vector);
		void operator+=(const Vector3& vector);
		void operator-=(const Vector3& vector);
		void operator*=(double scalar);
		void operator/=(double scalar);

		void SetComponents(double x, double y, double z);
		void GetComponents(double& x, double& y, double& z) const;

		void Add(const Vector3& leftVector, const Vector3& rightVector);
		void Subtract(const Vector3& leftVector, const Vector3& rightVector);

		void Cross(const Vector3& leftVector, const Vector3& rightVector);
		static double Dot(const Vector3& leftVector, const Vector3& rightVector);

		static double AngleBetween(const Vector3& vectorA, const Vector3& vectorB);

		double Length() const;
		void Scale(double scalar);

		bool Normalize();

		bool Projection(const Vector3& vectorA, const Vector3& vectorB);
		bool Rejection(const Vector3& vectorA, const Vector3& vectorB);
		bool Rotation(const Vector3& vector, const Vector3& axis, double angle);

		bool IsEqualTo(const Vector3& vector, double eps = FRUMPY_EPS) const;

		void Lerp(const Vector3& vectorA, const Vector3& vectorB, double alpha);
		void Slerp(const Vector3& vectorA, const Vector3& vectorB, double alpha);

		double x, y, z;
	};

	FRUMPY_API Vector3 operator+(const Vector3& leftVector, const Vector3& rightVector);
	FRUMPY_API Vector3 operator-(const Vector3& leftVector, const Vector3& rightVector);
	FRUMPY_API Vector3 operator*(const Vector3& vector, double scalar);
	FRUMPY_API Vector3 operator*(double scalar, const Vector3& vector);
	FRUMPY_API Vector3 operator/(const Vector3& vector, double scalar);
}