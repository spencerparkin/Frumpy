#pragma once

#include "Vector3.h"
#include "Plane.h"
#include "List.h"
#include "Matrix4x4.h"
#include "Aabb.h"
#include <vector>
#include <set>

namespace Frumpy
{
	class Mesh;
	class Frustum;
	class Triangle;

	class FRUMPY_API ConvexHull
	{
	public:
		ConvexHull();
		ConvexHull(const ConvexHull& convexHull);
		virtual ~ConvexHull();

		enum Polyhedron
		{
			REGULAR_TETRAHEDRON,
			REGULAR_HEXADRON,
			REGULAR_OCTAHEDRON,
			REGULAR_ICOSAHEDRON,
			REGULAR_DODECAHEDRON
		};

		bool Generate(const List<Vector3>& pointCloudList, bool compressFacets = false);
		bool Generate(const Frustum& frustum);
		bool Generate(const AxisAlignedBoundingBox& aabb);
		bool Generate(Polyhedron polyhedron, double uniformScale);
		Mesh* Generate(void) const;
		void Transform(const Matrix4x4& transformMatrix);
		bool OverlapsWith(const ConvexHull& convexHull, double eps = FRUMPY_EPS) const;
		bool OverlapsWith(const Triangle& triangle, double eps = FRUMPY_EPS) const;
		bool ContainsPoint(const Vector3& point, double eps = FRUMPY_EPS) const;
		bool AddPoint(const Vector3& point, double eps = FRUMPY_EPS);		// Return value indicates whether the hull expanded.
		const Vector3& GetPoint(int i) const;
		Vector3 CalcCenter() const;
		void CompressFacets();
		void RegenerateEdgeSetIfNecessary() const;
		bool LineSegmentHitsHull(const Vector3& pointA, const Vector3& pointB, double eps = FRUMPY_EPS) const;

	private:

		bool FindInitialTetrahedron(const List<Vector3>& pointCloudList);
		bool AnyPointInGivenConvexHull(const ConvexHull& convexHull, double eps) const;
		bool AnyEdgeHitsGivenConvexHull(const ConvexHull& convexHull, double eps) const;

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
		
		bool CompressFacetPair(const Facet& facetA, const Facet& facetB, Facet& compressedFacet);

		std::vector<Vector3>* pointArray;  // No 3 points should ever by co-linear.
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
	};
}