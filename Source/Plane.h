#pragma once

#include "Defines.h"
#include "Vector.h"

namespace Frumpy
{
	class Triangle;
	class Matrix;

	class FRUMPY_API Plane
	{
	public:
		Plane();
		Plane(const Vector& point, const Vector& normal);
		virtual ~Plane();

		bool SetFromPointAndVector(const Vector& point, const Vector& normal);
		bool GetToPointAndVector(Vector& point, Vector& normal) const;
		void SetFromTriangle(const Triangle& triangle);
		double SignedDistanceToPoint(const Vector& point) const;
		bool ContainsPoint(const Vector& point, double planeThickness) const;
		bool RayCast(const Vector& rayOrigin, const Vector& rayDirection, double& lambda) const;
		void Transform(const Matrix& transformMatrix, bool isRigidBodyTransform);
		bool IsEqualTo(const Plane& plane, double eps = FRUMPY_EPS) const;

		Vector unitNormal;
		double distance;
	};
}