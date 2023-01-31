#pragma once

#include "Defines.h"
#include "Vector.h"

namespace Frumpy
{
	class FRUMPY_API AxisAlignedBoundingBox
	{
	public:
		AxisAlignedBoundingBox();
		AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb);
		virtual ~AxisAlignedBoundingBox();

		void operator=(const AxisAlignedBoundingBox& aabb);

		bool IsValid() const;
		void Invalidate();
		bool ContainsPoint(const Vector& point, double eps = 0.0) const;
		void MinimallyExpandToContainPoint(const Vector& point);
		void MinimallyExpandToContainBox(const AxisAlignedBoundingBox& aabb);
		double Width() const;
		double Height() const;
		double Depth() const;
		double Volume() const;
		void Split(AxisAlignedBoundingBox& aabb0, AxisAlignedBoundingBox& aabb1) const;
		bool Intersect(const AxisAlignedBoundingBox& aabb0, const AxisAlignedBoundingBox& aabb1);
		Vector Center() const;

		static bool IntersectIntervals(double minA, double maxA, double minB, double maxB, double& minC, double& maxC);

		Vector min, max;
	};
}