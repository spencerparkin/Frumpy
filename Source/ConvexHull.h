#pragma once

#include "Vector.h"
#include "Plane.h"
#include "List.h"
#include "Matrix.h"
#include "Camera.h"
#include "Aabb.h"
#include <vector>
#include <set>

namespace Frumpy
{
	class Mesh;

	class FRUMPY_API ConvexHull
	{
	public:
		ConvexHull();
		virtual ~ConvexHull();

		enum Polyhedron
		{
			REGULAR_TETRAHEDRON,
			REGULAR_HEXADRON,
			REGULAR_OCTAHEDRON,
			REGULAR_ICOSAHEDRON,
			REGULAR_DODECAHEDRON
		};

		bool Generate(const List<Vector>& pointCloudList, bool compressFacets = false);
		bool Generate(const Camera::Frustum& frustum);
		bool Generate(const AxisAlignedBoundingBox& aabb);
		bool Generate(Polyhedron polyhedron, double uniformScale);
		Mesh* Generate(void) const;
		void Transform(const Matrix& transformMatrix);
		bool OverlapsWith(const ConvexHull& convexHull, double eps = FRUMPY_EPS) const;
		bool ContainsPoint(const Vector& point, double eps = FRUMPY_EPS) const;
		bool AddPoint(const Vector& point, double eps = FRUMPY_EPS);		// Return value indicates whether the hull expanded.
		const Vector& GetPoint(int i) const;
		Vector CalcCenter() const;
		void CompressFacets();

	private:

		bool FindInitialTetrahedron(const List<Vector>& pointCloudList);
		bool AnyPointInGivenConvexHull(const ConvexHull& convexHull, double eps) const;
		bool AnyEdgeStraddlesGivenConvexHull(const ConvexHull& convexHull, double eps) const;

		struct Facet	// Always a convex polygon with non-zero area.
		{
			Facet();
			Facet(const Facet& facet);

			std::vector<int> pointArray;	// Always in CCW order when viewing the hull from the outside.
			mutable Plane cachedPlane;
			mutable bool cachedPlaneValid;

			const Plane& GetSurfacePlane(const ConvexHull& convexHull) const;
			bool IsCanceledBy(const Facet& facet) const;
			bool Tessellate(const ConvexHull& convexHull, unsigned int*& triangleBuffer) const;
			int FindPoint(int i) const;
		};
		
		bool CompressFacetPair(Facet& facetA, Facet& facetB, Facet& compressedFacet);

		std::vector<Vector>* pointArray;  // No 3 points should ever by co-linear.
		std::vector<Facet>* facetArray;

		struct Edge
		{
			int i, j;

			bool operator<(const Edge& edge) const
			{
				int64_t keyA = this->CalcKey();
				int64_t keyB = edge.CalcKey();
				return keyA < keyB;
			}

			int64_t CalcKey() const
			{
				return (this->i < this->j) ? ((int64_t(this->i) << 32) | int64_t(this->j)) : ((int64_t(this->j) << 32) | int64_t(this->i));
			}
		};

		mutable std::set<Edge>* cachedEdgeSet;
		mutable bool cachedEdgeSetValid;

		void RegenerateEdgeSetIfNecessary() const;
	};
}