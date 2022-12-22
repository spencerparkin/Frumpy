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

		// Assuming the given point is on this triangle, calculate its barycentric coordinates.
		bool CalcBarycentricCoordinates(const Vector& interiorPoint, Vector& baryCoords) const;

		Vector vertex[3];
	};
}