#include "ConvexHull.h"
#include "Triangle.h"
#include <math.h>

using namespace Frumpy;

ConvexHull::ConvexHull()
{
	this->pointArray = new std::vector<Vector>();
	this->facetArray = new std::vector<Facet>();
	this->cachedEdgeSet = new std::set<Edge>();
	this->cachedEdgeSetValid = false;
}

/*virtual*/ ConvexHull::~ConvexHull()
{
	delete this->pointArray;
	delete this->facetArray;
	delete this->cachedEdgeSet;
}

bool ConvexHull::Generate(const List<Vector>& pointCloudList)
{
	if (pointCloudList.GetCount() < 4)
		return false;

	if (!this->FindInitialTetrahedron(pointCloudList))
		return false;

	for (const List<Vector>::Node* node = pointCloudList.GetHead(); node; node = node->GetNext())
		this->AddPoint(node->value);

	return true;
}

bool ConvexHull::AddPoint(const Vector& point, double eps /*= FRUMPY_EPS*/)
{
	if (this->ContainsPoint(point, eps))
		return false;
	
	this->cachedEdgeSetValid = false;
	this->pointArray->push_back(point);

	// Generate a new set of facets from the given point.
	std::vector<Facet> newFacetArray;
	for (int i = 0; i < (signed)this->facetArray->size(); i++)
	{
		const Facet& oldFacet = (*this->facetArray)[i];
		
		const Vector& pointA = this->GetPoint(oldFacet.pointArray[0]);
		const Vector& pointB = this->GetPoint(oldFacet.pointArray[1]);
		const Vector& pointC = this->GetPoint(oldFacet.pointArray[2]);
		
		Vector edgeX = pointA - point;
		Vector edgeY = pointB - point;
		Vector edgeZ = pointC - point;

		Vector crossProduct;
		crossProduct.Cross(edgeX, edgeY);
		double dotProduct = Vector::Dot(crossProduct, edgeZ);

		if (dotProduct < 0.0)
		{
			for (int j = 0; j < oldFacet.pointArray.size(); j++)
			{
				Facet newSideFacet;
				newSideFacet.pointArray.push_back(this->pointArray->size() - 1);
				newSideFacet.pointArray.push_back(oldFacet.pointArray[j]);
				newSideFacet.pointArray.push_back(oldFacet.pointArray[(j + 1) % oldFacet.pointArray.size()]);
				newFacetArray.push_back(newSideFacet);
			}

			Facet newBaseFacet;
			for (int j = oldFacet.pointArray.size() - 1; j >= 0; j--)
				newBaseFacet.pointArray.push_back(oldFacet.pointArray[j]);

			newFacetArray.push_back(newBaseFacet);
		}
	}

	// Integrate the new facets with our existing ones.  This means we
	// add the facet to our list if it does not already exist there mirrored.
	// If it does already, then the two facets cancel one another out.
	while (newFacetArray.size() > 0)
	{
		Facet& newFacet = newFacetArray[newFacetArray.size() - 1];

		bool canceled = false;
		for (int i = 0; i < (signed)this->facetArray->size(); i++)
		{
			Facet& oldFacet = (*this->facetArray)[i];
			if (oldFacet.IsCanceledBy(newFacet))
			{
				this->facetArray->erase(this->facetArray->begin() + i);
				canceled = true;
				break;
			}
		}

		if (!canceled)
			this->facetArray->push_back(newFacet);

		newFacetArray.pop_back();
	}

	return true;
}

bool ConvexHull::FindInitialTetrahedron(const List<Vector>& pointCloudList)
{
	this->pointArray->clear();
	this->facetArray->clear();

	// Find an initial tetrahedron from the given point-cloud.
	// If the given list contained a billion points, all co-planar, co-linear, or all the same point, then
	// this would take forever before we realized no hull could be generated.
	for (const List<Vector>::Node* nodeA = pointCloudList.GetHead(); nodeA; nodeA = nodeA->GetNext())
	{
		const Vector& pointA = nodeA->value;

		for (const List<Vector>::Node* nodeB = nodeA->GetNext(); nodeB; nodeB = nodeB->GetNext())
		{
			const Vector& pointB = nodeB->value;

			for (const List<Vector>::Node* nodeC = nodeB->GetNext(); nodeC; nodeC = nodeC->GetNext())
			{
				const Vector& pointC = nodeC->value;

				for (const List<Vector>::Node* nodeD = nodeC->GetNext(); nodeD; nodeD = nodeD->GetNext())
				{
					const Vector& pointD = nodeD->value;

					Vector edgeX = pointB - pointA;
					Vector edgeY = pointC - pointA;
					Vector edgeZ = pointD - pointA;

					Vector crossProduct;
					crossProduct.Cross(edgeX, edgeY);
					double dotProduct = Vector::Dot(crossProduct, edgeZ);

					if (dotProduct > 0.0)
					{
						this->pointArray->push_back(pointA);
						this->pointArray->push_back(pointB);
						this->pointArray->push_back(pointC);
						this->pointArray->push_back(pointD);

						Facet facet[4];

						facet[0].pointArray.push_back(0);
						facet[0].pointArray.push_back(2);
						facet[0].pointArray.push_back(1);

						facet[1].pointArray.push_back(0);
						facet[1].pointArray.push_back(3);
						facet[1].pointArray.push_back(2);

						facet[2].pointArray.push_back(0);
						facet[2].pointArray.push_back(1);
						facet[2].pointArray.push_back(3);
						
						facet[3].pointArray.push_back(1);
						facet[3].pointArray.push_back(2);
						facet[3].pointArray.push_back(3);

						this->facetArray->push_back(facet[0]);
						this->facetArray->push_back(facet[1]);
						this->facetArray->push_back(facet[2]);
						this->facetArray->push_back(facet[3]);

						return true;
					}
				}
			}
		}
	}

	return false;
}

bool ConvexHull::Generate(const Camera::Frustum& frustum)
{
	double hfoviRads = FRUMPY_DEGS_TO_RADS(frustum.hfovi);
	double vfoviRads = FRUMPY_DEGS_TO_RADS(frustum.vfovi);

	double nearX = tan(hfoviRads / 2.0) * frustum._near;
	double farX = tan(hfoviRads / 2.0) * frustum._far;
	double nearY = tan(vfoviRads / 2.0) * frustum._near;
	double farY = tan(vfoviRads / 2.0) * frustum._far;
	
	List<Vector> pointList;

	pointList.AddTail(Vector(-nearX, -nearY, -frustum._near));
	pointList.AddTail(Vector(nearX, -nearY, -frustum._near));
	pointList.AddTail(Vector(-nearX, nearY, -frustum._near));
	pointList.AddTail(Vector(nearX, nearY, -frustum._near));

	pointList.AddTail(Vector(-farX, -farY, -frustum._far));
	pointList.AddTail(Vector(farX, -farY, -frustum._far));
	pointList.AddTail(Vector(-farX, farY, -frustum._far));
	pointList.AddTail(Vector(farX, farY, -frustum._far));

	return this->Generate(pointList);
}

bool ConvexHull::Generate(const AxisAlignedBoundingBox& aabb)
{
	List<Vector> pointList;

	pointList.AddTail(aabb.min);
	pointList.AddTail(Vector(aabb.min.x, aabb.min.y, aabb.max.z));
	pointList.AddTail(Vector(aabb.min.x, aabb.max.y, aabb.min.z));
	pointList.AddTail(Vector(aabb.min.x, aabb.max.y, aabb.max.z));
	pointList.AddTail(Vector(aabb.max.x, aabb.min.y, aabb.min.z));
	pointList.AddTail(Vector(aabb.max.x, aabb.min.y, aabb.max.z));
	pointList.AddTail(Vector(aabb.max.x, aabb.max.y, aabb.min.z));
	pointList.AddTail(aabb.max);

	return this->Generate(pointList);
}

void ConvexHull::Transform(const Matrix& transformMatrix, bool isRigidBodyTransform)
{
	for (int i = 0; i < (signed)this->pointArray->size(); i++)
	{
		Vector& point = (*this->pointArray)[i];
		transformMatrix.TransformPoint(point, point);
	}

	for (int i = 0; i < (signed)this->facetArray->size(); i++)
	{
		Facet& facet = (*this->facetArray)[i];
		facet.cachedPlaneValid = false;
	}
}

bool ConvexHull::AnyPointInGivenConvexHull(const ConvexHull& convexHull) const
{
	for (int i = 0; i < (signed)this->pointArray->size(); i++)
		if (convexHull.ContainsPoint((*this->pointArray)[i]))
			return true;

	return false;
}

bool ConvexHull::AnyEdgeStraddlesGivenConvexHull(const ConvexHull& convexHull) const
{
	this->RegenerateEdgeSetIfNecessary();

	for (const Edge& edge : *this->cachedEdgeSet)
	{
		const Vector& pointA = this->GetPoint(edge.i);
		const Vector& pointB = this->GetPoint(edge.j);

		for (int i = 0; i < (signed)convexHull.facetArray->size(); i++)
		{
			const Facet& facet = (*convexHull.facetArray)[i];
			const Plane& plane = facet.GetSurfacePlane(convexHull);
			double distanceA = plane.SignedDistanceToPoint(pointA);
			double distanceB = plane.SignedDistanceToPoint(pointB);
			if (FRUMPY_SIGN(distanceA) != FRUMPY_SIGN(distanceB))
				return true;
		}
	}

	return false;
}

bool ConvexHull::OverlapsWith(const ConvexHull& convexHull) const
{
	if (this->AnyPointInGivenConvexHull(convexHull))
		return true;

	if (convexHull.AnyPointInGivenConvexHull(*this))
		return true;

	if (this->AnyEdgeStraddlesGivenConvexHull(convexHull))
		return true;

	if (convexHull.AnyEdgeStraddlesGivenConvexHull(*this))
		return true;

	return false;
}

bool ConvexHull::ContainsPoint(const Vector& point, double eps /*= FRUMPY_EPS*/) const
{
	for (int i = 0; i < (signed)this->facetArray->size(); i++)
	{
		const Facet& facet = (*this->facetArray)[i];
		const Plane& plane = facet.GetSurfacePlane(*this);
		double distance = plane.SignedDistanceToPoint(point);
		if (distance > FRUMPY_EPS)
			return false;
	}

	return true;
}

void ConvexHull::RegenerateEdgeSetIfNecessary() const
{
	if (this->cachedEdgeSetValid)
		return;

	this->cachedEdgeSet->clear();

	for (int i = 0; i < (signed)this->facetArray->size(); i++)
	{
		const Facet& facet = (*this->facetArray)[i];
		for (int j = 0; j < (signed)facet.pointArray.size(); j++)
		{
			Edge edge;
			edge.i = facet.pointArray[j];
			edge.j = facet.pointArray[(j + 1) % facet.pointArray.size()];
			if (this->cachedEdgeSet->find(edge) == this->cachedEdgeSet->end())
				this->cachedEdgeSet->insert(edge);
		}
	}

	this->cachedEdgeSetValid = true;
}

const Vector& ConvexHull::GetPoint(int i) const
{
	i = i % this->pointArray->size();
	if (i < 0)
		i += this->pointArray->size();
	return (*this->pointArray)[i];
}

ConvexHull::Facet::Facet()
{
	this->cachedPlaneValid = false;
}

ConvexHull::Facet::Facet(const Facet& facet)
{
	this->pointArray = facet.pointArray;
	this->cachedPlaneValid = false;
}

const Plane& ConvexHull::Facet::GetSurfacePlane(const ConvexHull& convexHull) const
{
	if (!this->cachedPlaneValid)
	{
		Triangle triangle;
		triangle.vertex[0] = convexHull.GetPoint(this->pointArray[0]);
		triangle.vertex[1] = convexHull.GetPoint(this->pointArray[1]);
		triangle.vertex[2] = convexHull.GetPoint(this->pointArray[2]);
		this->cachedPlane.SetFromTriangle(triangle);
		this->cachedPlaneValid = true;
	}

	return this->cachedPlane;
}

bool ConvexHull::Facet::IsCanceledBy(const Facet& facet) const
{
	if (this->pointArray.size() != facet.pointArray.size())
		return false;

	for (int i = 0; i < (signed)this->pointArray.size(); i++)
	{
		bool canceled = true;

		for (int j = 0; j < (signed)this->pointArray.size(); j++)
		{
			int u = this->pointArray.size() - 1 - j;
			int v = (j + i) % this->pointArray.size();

			if (this->pointArray[u] != facet.pointArray[v])
			{
				canceled = false;
				break;
			}
		}

		if (canceled)
			return true;
	}

	return false;
}