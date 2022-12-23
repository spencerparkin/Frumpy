#include "Triangle.h"
#include <float.h>

using namespace Frumpy;

Triangle::Triangle()
{
}

/*virtual*/ Triangle::~Triangle()
{
}

bool Triangle::CalcBarycentricCoordinates(const Vector& interiorPoint, Vector& baryCoords) const
{
	Vector xAxis = this->vertex[1] - this->vertex[0];
	if (!xAxis.Normalize())
		return false;

	Vector yAxis;
	if (!yAxis.Rejection(this->vertex[2] - this->vertex[0], xAxis))
		return false;

	Vector point = interiorPoint - this->vertex[0];
	Vector edgeU = this->vertex[1] - this->vertex[0];
	Vector edgeV = this->vertex[2] - this->vertex[0];

	double px = Vector::Dot(point, xAxis);
	double py = Vector::Dot(point, yAxis);
	double ux = Vector::Dot(edgeU, xAxis);
	double uy = Vector::Dot(edgeU, yAxis);
	double vx = Vector::Dot(edgeV, xAxis);
	double vy = Vector::Dot(edgeV, yAxis);

	double det = ux * vy - uy * vx;
	if (det == 0.0)
		return false;

	double alpha = (px * vy - py * vx) / det;
	double beta = (-px * uy + py * ux) / det;

	baryCoords.x = FRUMPY_CLAMP(1.0 - alpha - beta, 0.0, 1.0);
	baryCoords.y = FRUMPY_CLAMP(alpha, 0.0, 1.0);
	baryCoords.z = FRUMPY_CLAMP(beta, 0.0, 1.0);

	return true;
}

double Triangle::Area() const
{
	Vector product;
	product.Cross(this->vertex[1] - this->vertex[0], this->vertex[2] - this->vertex[0]);
	return product.Length() / 2.0;
}

double Triangle::SmallestInteriorAngle() const
{
	double smallestAngle = FLT_MAX;
	for (int i = 0; i < 3; i++)
	{
		const Vector& vertexA = this->vertex[i];
		const Vector& vertexB = this->vertex[(i + 1) % 3];
		const Vector& vertexC = this->vertex[(i + 2) % 3];

		double angle = Vector::AngleBetween(vertexB - vertexA, vertexC - vertexA);
		if (angle < smallestAngle)
			smallestAngle = angle;
	}

	return smallestAngle;
}