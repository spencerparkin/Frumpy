#include "Aabb.h"
#include <float.h>

using namespace Frumpy;

AxisAlignedBoundingBox::AxisAlignedBoundingBox()
{
	this->Invalidate();
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox(const AxisAlignedBoundingBox& aabb)
{
	this->min = aabb.min;
	this->max = aabb.max;
}

/*virtual*/ AxisAlignedBoundingBox::~AxisAlignedBoundingBox()
{
}

bool AxisAlignedBoundingBox::IsValid() const
{
	return this->min.x <= this->max.x &&
			this->min.y <= this->max.y &&
			this->min.z <= this->max.z;
}

void AxisAlignedBoundingBox::Invalidate()
{
	this->min.SetComponents(FLT_MAX, FLT_MAX, FLT_MAX);
	this->max.SetComponents(FLT_MIN, FLT_MIN, FLT_MIN);
}

void AxisAlignedBoundingBox::operator=(const AxisAlignedBoundingBox& aabb)
{
	this->min = aabb.min;
	this->max = aabb.max;
}

bool AxisAlignedBoundingBox::ContainsPoint(const Vector& point, double eps /*= 0.0*/) const
{
	return (-eps + this->min.x <= point.x && point.x <= this->max.x + eps) &&
			(-eps + this->min.y <= point.y && point.y <= this->max.y + eps) &&
			(-eps + this->min.z <= point.z && point.z <= this->max.z + eps);
}

void AxisAlignedBoundingBox::MinimallyExpandToContainPoint(const Vector& point)
{
	if (!this->IsValid())
		this->min = this->max = point;
	else
	{
		this->min.x = FRUMPY_MIN(this->min.x, point.x);
		this->min.y = FRUMPY_MIN(this->min.y, point.y);
		this->min.z = FRUMPY_MIN(this->min.z, point.z);

		this->max.x = FRUMPY_MAX(this->max.x, point.x);
		this->max.y = FRUMPY_MAX(this->max.y, point.y);
		this->max.z = FRUMPY_MAX(this->max.z, point.z);
	}
}

void AxisAlignedBoundingBox::MinimallyExpandToContainBox(const AxisAlignedBoundingBox& aabb)
{
	this->MinimallyExpandToContainPoint(aabb.min);
	this->MinimallyExpandToContainPoint(aabb.max);
}

double AxisAlignedBoundingBox::Width() const
{
	return this->max.x - this->min.x;
}

double AxisAlignedBoundingBox::Height() const
{
	return this->max.y - this->min.y;
}

double AxisAlignedBoundingBox::Depth() const
{
	return this->max.z - this->min.z;
}

double AxisAlignedBoundingBox::Volume() const
{
	return this->Width() * this->Height() * this->Depth();
}

void AxisAlignedBoundingBox::Split(AxisAlignedBoundingBox& aabb0, AxisAlignedBoundingBox& aabb1) const
{
	double width = this->Width();
	double height = this->Height();
	double depth = this->Depth();

	aabb0 = *this;
	aabb1 = *this;

	if (width > height && width > depth)
	{
		aabb0.max.x = aabb1.min.x = this->min.x + 0.5 * width;
	}
	else if (height > width && height > depth)
	{
		aabb0.max.y = aabb1.min.y = this->min.y + 0.5 * height;
	}
	else
	{
		aabb0.max.z = aabb1.min.z = this->min.z + 0.5 * depth;
	}
}

bool AxisAlignedBoundingBox::Intersect(const AxisAlignedBoundingBox& aabb0, const AxisAlignedBoundingBox& aabb1)
{
	if (!IntersectIntervals(aabb0.min.x, aabb0.max.x, aabb1.min.x, aabb1.max.x, this->min.x, this->max.x))
		return false;

	if (!IntersectIntervals(aabb0.min.y, aabb0.max.y, aabb1.min.y, aabb1.max.y, this->min.y, this->max.y))
		return false;

	if (!IntersectIntervals(aabb0.min.z, aabb0.max.z, aabb1.min.z, aabb1.max.z, this->min.z, this->max.z))
		return false;

	return true;
}

/*static*/ bool AxisAlignedBoundingBox::IntersectIntervals(double minA, double maxA, double minB, double maxB, double& minC, double& maxC)
{
	if (minA > maxA)
		return false;

	if (minB > maxB)
		return false;

	if ((minA <= minB && minB <= maxA) ||
		(minA <= maxB && maxB <= maxA) ||
		(minB <= minA && minA <= maxB) ||
		(minB <= maxA && maxA <= maxB))
	{
		minC = FRUMPY_MAX(minA, minB);
		maxC = FRUMPY_MIN(maxA, maxB);
	}

	return false;
}

Vector AxisAlignedBoundingBox::Center() const
{
	return this->min + (this->max - this->min) * 0.5;
}