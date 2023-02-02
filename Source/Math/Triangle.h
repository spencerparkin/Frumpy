#pragma once

#include "Defines.h"
#include "Math/Vector3.h"

namespace Frumpy
{
	class FRUMPY_API Triangle
	{
	public:
		Triangle();
		virtual ~Triangle();

		bool CalcBarycentricCoordinates(const Vector3& interiorPoint, Vector3& baryCoords) const;
		double Area() const;
		double SmallestInteriorAngle() const;
		bool CalcNormal(Vector3& normal) const;
		bool IsEqualTo(const Triangle& triangle, double eps = 1e-4) const;

		Vector3 vertex[3];
	};
}