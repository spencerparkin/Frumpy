#include "Math/Triangle.h"
#include <float.h>

using namespace Frumpy;

Triangle::Triangle()
{
}

/*virtual*/ Triangle::~Triangle()
{
}

bool Triangle::CalcBarycentricCoordinates(const Vector3& interiorPoint, Vector3& baryCoords) const
{
	Vector3 edgeU = this->vertex[1] - this->vertex[0];
	Vector3 edgeV = this->vertex[2] - this->vertex[0];
	Vector3 point = interiorPoint - this->vertex[0];

	Vector3 xAxis = edgeU;
	if (!xAxis.Normalize())
		return false;

	Vector3 yAxis = edgeV;
	if (!yAxis.Normalize())
		return false;

	yAxis = yAxis - xAxis * Vector3::Dot(yAxis, xAxis);
	if (!yAxis.Normalize())
		return false;

	double px = Vector3::Dot(point, xAxis);
	double py = Vector3::Dot(point, yAxis);
	double ux = Vector3::Dot(edgeU, xAxis);
	double uy = Vector3::Dot(edgeU, yAxis);
	double vx = Vector3::Dot(edgeV, xAxis);
	double vy = Vector3::Dot(edgeV, yAxis);

	double det = ux * vy - uy * vx;
	if (det == 0.0)
		return false;

	double alpha = (px * vy - py * vx) / det;
	double beta = (-px * uy + py * ux) / det;
	//if (alpha < 0.0 || alpha > 1.0 || beta < 0.0 || beta > 1.0)
	//	return false;

	alpha = FRUMPY_CLAMP(alpha, 0.0, 1.0);
	beta = FRUMPY_CLAMP(beta, 0.0, 1.0);

	baryCoords.y = alpha;
	baryCoords.z = beta;
	baryCoords.x = 1.0 - alpha - beta;

	return true;
}

double Triangle::Area() const
{
	Vector3 product;
	product.Cross(this->vertex[1] - this->vertex[0], this->vertex[2] - this->vertex[0]);
	return product.Length() / 2.0;
}

double Triangle::SmallestInteriorAngle() const
{
	double smallestAngle = FLT_MAX;
	for (int i = 0; i < 3; i++)
	{
		const Vector3& vertexA = this->vertex[i];
		const Vector3& vertexB = this->vertex[(i + 1) % 3];
		const Vector3& vertexC = this->vertex[(i + 2) % 3];

		double angle = Vector3::AngleBetween(vertexB - vertexA, vertexC - vertexA);
		if (angle < smallestAngle)
			smallestAngle = angle;
	}

	return smallestAngle;
}

bool Triangle::CalcNormal(Vector3& normal) const
{
	normal.Cross(this->vertex[1] - this->vertex[0], this->vertex[2] - this->vertex[0]);
	return normal.Normalize();
}

bool Triangle::IsEqualTo(const Triangle& triangle, double eps /*= 1e-4*/) const
{
	for (int i = 0; i < 3; i++)
	{
		if (this->vertex[0].IsEqualTo(triangle.vertex[i], eps) &&
			this->vertex[1].IsEqualTo(triangle.vertex[(i + 1) % 3], eps) &&
			this->vertex[2].IsEqualTo(triangle.vertex[(i + 2) % 3], eps))
		{
			return true;
		}
	}

	return false;
}