#pragma once

#include "Defines.h"
#include "Vector3.h"

namespace Frumpy
{
	class Triangle;
	class Matrix4x4;

	class FRUMPY_API Plane
	{
	public:
		Plane();
		Plane(const Vector3& point, const Vector3& normal);
		virtual ~Plane();

		bool SetFromPointAndVector(const Vector3& point, const Vector3& normal);
		bool GetToPointAndVector(Vector3& point, Vector3& normal) const;
		void SetFromTriangle(const Triangle& triangle);
		double SignedDistanceToPoint(const Vector3& point) const;
		bool ContainsPoint(const Vector3& point, double planeThickness) const;
		bool RayCast(const Vector3& rayOrigin, const Vector3& rayDirection, double& lambda) const;
		void Transform(const Matrix4x4& transformMatrix, bool isRigidBodyTransform);
		bool IsEqualTo(const Plane& plane, double eps = FRUMPY_EPS) const;

		Vector3 unitNormal;
		double distance;
	};
}