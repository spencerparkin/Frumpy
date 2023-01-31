#pragma once

#include "Vector.h"
#include "Plane.h"
#include "List.h"
#include "Matrix.h"
#include "Camera.h"
#include "Aabb.h"
#include <vector>

namespace Frumpy
{
	class FRUMPY_API ConvexHull
	{
	public:
		ConvexHull();
		virtual ~ConvexHull();

		bool Generate(const List<Vector>& pointCloudList);
		bool Generate(const Camera::Frustum& frustum);
		bool Generate(const AxisAlignedBoundingBox& aabb);
		void Transform(const Matrix& transformMatrix, bool isRigidBodyTransform);
		bool OverlapsWith(const ConvexHull& convexHull) const;
		bool ContainsPoint(const Vector& point) const;
		bool AddPoint(const Vector& point);		// Return value indicates whether the hull expanded.

	private:

		bool FindInitialTetrahedron(const List<Vector>& pointCloudList);

		struct Facet	// Always a convex polygon with non-zero area.
		{
			std::vector<int> pointArray;	// Always in CCW order when viewing the hull from the outside.

			bool IsCanceledBy(const Facet& facet) const;
		};
		
		std::vector<Vector>* pointArray;
		std::vector<Facet>* facetArray;
	};
}