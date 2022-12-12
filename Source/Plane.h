#pragma once

#include "Defines.h"
#include "Vector.h"

namespace Frumpy
{
	class FRUMPY_API Plane
	{
	public:
		Plane();
		Plane(const Vector& point, const Vector& normal);
		virtual ~Plane();

		bool SetFromPointAndVector(const Vector& point, const Vector& normal);
		bool GetToPointAndVector(Vector& point, Vector& normal) const;

		double SignedDistanceToPoint(const Vector& point) const;

		bool ContainsPoint(const Vector& point, double planeThickness) const;

		Vector unitNormal;
		double distance;
	};
}