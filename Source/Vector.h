#pragma once

#include "Defines.h"

namespace Frumpy
{
	// These are 3-dimensional vectors geared toward 3D computer graphics applications.
	class FRUMPY_API Vector
	{
	public:
		Vector();
		Vector(double x, double y, double z);
		Vector(const Vector& vector);
		virtual ~Vector();

		void operator=(const Vector& vector);

		void SetComponents(double x, double y, double z);
		void GetComponents(double& x, double& y, double& z) const;

		void Add(const Vector& leftVector, const Vector& rightVector);
		void Subtract(const Vector& leftVector, const Vector& rightVector);

		void Cross(const Vector& leftVector, const Vector& rightVector);
		static double Dot(const Vector& leftVector, const Vector& rightVector);

		double Length() const;
		void Scale(double scalar);

		bool Normalize();

		bool Projection(const Vector& vectorA, const Vector& vectorB);
		bool Rejection(const Vector& vectorA, const Vector& vectorB);
		bool Rotation(const Vector& vector, const Vector& axis, double angle);

		double x, y, z;
	};

	FRUMPY_API Vector operator+(const Vector& leftVector, const Vector& rightVector);
	FRUMPY_API Vector operator-(const Vector& leftVector, const Vector& rightVector);
	FRUMPY_API Vector operator*(const Vector& vector, double scalar);
	FRUMPY_API Vector operator*(double scalar, const Vector& vector);
}