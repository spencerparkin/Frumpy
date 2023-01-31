#include "ConvexHull.h"
#include <math.h>

using namespace Frumpy;

ConvexHull::ConvexHull()
{
	this->pointArray = new std::vector<Vector>();
	this->facetArray = new std::vector<Facet>();
}

/*virtual*/ ConvexHull::~ConvexHull()
{
	delete this->pointArray;
	delete this->facetArray;
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

bool ConvexHull::AddPoint(const Vector& point)
{
	if (this->ContainsPoint(point))
		return false;
	
	this->pointArray->push_back(point);

	std::vector<Facet> newFacetArray;
	// TODO: Generate new facets here.

	// Integrate the new facets with our existing ones.  This means we
	// add the facet to our list if it does not already exist there mirrored.
	// If it does already, then the two facets cancel one another out.
	for (int i = 0; i < (signed)this->facetArray->size(); i++)
	{
		Facet& oldFacet = (*this->facetArray)[i];
		for (int j = 0; j < (signed)newFacetArray.size(); j++)
		{
			Facet& newFacet = newFacetArray[j];
			if (oldFacet.IsCanceledBy(newFacet))
			{
				this->facetArray->erase(this->facetArray->begin() + i);
				newFacetArray.erase(newFacetArray.begin() + j);
				i--;
				break;
			}
		}
	}

	for (int i = 0; i < (signed)newFacetArray.size(); i++)
		this->facetArray->push_back(newFacetArray[i]);

	return true;
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

						Facet facet;
						facet.pointArray.push_back(0);
						facet.pointArray.push_back(2);
						facet.pointArray.push_back(1);
						this->facetArray->push_back(facet);

						facet.pointArray.clear();
						facet.pointArray.push_back(0);
						facet.pointArray.push_back(3);
						facet.pointArray.push_back(2);
						this->facetArray->push_back(facet);

						facet.pointArray.clear();
						facet.pointArray.push_back(0);
						facet.pointArray.push_back(1);
						facet.pointArray.push_back(3);
						this->facetArray->push_back(facet);

						facet.pointArray.clear();
						facet.pointArray.push_back(1);
						facet.pointArray.push_back(2);
						facet.pointArray.push_back(3);
						this->facetArray->push_back(facet);

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
	/*
	for (List<Vector>::Node* pointNode = this->pointList.GetHead(); pointNode; pointNode = pointNode->GetNext())
		transformMatrix.TransformPoint(pointNode->value, pointNode->value);

	for (List<Plane>::Node* planeNode = this->planeList.GetHead(); planeNode; planeNode = planeNode->GetNext())
		planeNode->value.Transform(transformMatrix, isRigidBodyTransform);
	*/
}

bool ConvexHull::OverlapsWith(const ConvexHull& convexHull) const
{
	/*
	for (const List<Vector>::Node* pointNode = this->pointList.GetHead(); pointNode; pointNode = pointNode->GetNext())
		if (convexHull.ContainsPoint(pointNode->value))
			return true;

	for (const List<Vector>::Node* pointNode = convexHull.pointList.GetHead(); pointNode; pointNode = pointNode->GetNext())
		if (this->ContainsPoint(pointNode->value))
			return true;

	for (const List<Edge>::Node* edgeNode = this->edgeList.GetHead(); edgeNode; edgeNode = edgeNode->GetNext())
	{
		for (const List<Plane>::Node* planeNode = convexHull.planeList.GetHead(); planeNode; planeNode = planeNode->GetNext())
		{
			double distanceA = planeNode->value.SignedDistanceToPoint(*edgeNode->value.pointA);
			double distanceB = planeNode->value.SignedDistanceToPoint(*edgeNode->value.pointB);
			if (FRUMPY_SIGN(distanceA) != FRUMPY_SIGN(distanceB))
				return true;
		}
	}

	for (const List<Edge>::Node* edgeNode = convexHull.edgeList.GetHead(); edgeNode; edgeNode = edgeNode->GetNext())
	{
		for (const List<Plane>::Node* planeNode = this->planeList.GetHead(); planeNode; planeNode = planeNode->GetNext())
		{
			double distanceA = planeNode->value.SignedDistanceToPoint(*edgeNode->value.pointA);
			double distanceB = planeNode->value.SignedDistanceToPoint(*edgeNode->value.pointB);
			if (FRUMPY_SIGN(distanceA) != FRUMPY_SIGN(distanceB))
				return true;
		}
	}
	*/

	return false;
}

bool ConvexHull::ContainsPoint(const Vector& point) const
{
	/*
	for (const List<Plane>::Node* planeNode = this->planeList.GetHead(); planeNode; planeNode = planeNode->GetNext())
	{
		double distance = planeNode->value.SignedDistanceToPoint(point);
		if (distance > 0.0)
			return false;
	}
	*/

	return true;
}