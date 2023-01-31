#pragma once

#include "Defines.h"
#include "Vector.h"

namespace Frumpy
{
	class FRUMPY_API Triangle
	{
	public:
		Triangle();
		virtual ~Triangle();

		bool CalcBarycentricCoordinates(const Vector& interiorPoint, Vector& baryCoords) const;
		double Area() const;
		double SmallestInteriorAngle() const;
		bool CalcNormal(Vector& normal) const;
		bool IsEqualTo(const Triangle& triangle, double eps = 1e-4) const;

		Vector vertex[3];
	};
}