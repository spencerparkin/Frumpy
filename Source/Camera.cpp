#include "Camera.h"
#include <math.h>

using namespace Frumpy;

Camera::Camera()
{
	this->worldTransform.Identity();
}

/*virtual*/ Camera::~Camera()
{
}

bool Camera::LookAt(const Vector& eyePoint, const Vector& eyeTarget, const Vector& upDirection)
{
	Vector zAxis = eyePoint - eyeTarget;
	if (!zAxis.Normalize())
		return false;

	Vector yAxis;
	if (!yAxis.Rejection(upDirection, zAxis))
		return false;

	if (!yAxis.Normalize())
		return false;

	Vector xAxis;
	xAxis.Cross(yAxis, zAxis);

	this->worldTransform.Identity();
	this->worldTransform.SetCol(0, xAxis);
	this->worldTransform.SetCol(1, yAxis);
	this->worldTransform.SetCol(2, zAxis);
	this->worldTransform.SetCol(3, eyePoint);

	return true;
}

Frustum::Frustum()
{
	this->_near = 0.1;
	this->_far = 1000.0;
	this->hfovi = 60.0;
	this->vfovi = 50.0;
	this->frustumHullValid = false;
}

/*virtual*/ Frustum::~Frustum()
{
}

void Frustum::AdjustVFoviForAspectRatio(double aspectRatio)
{
	this->vfovi = FRUMPY_RADS_TO_DEGS(2.0 * atan(tan(FRUMPY_DEGS_TO_RADS(this->hfovi) / 2.0) / aspectRatio));
	this->frustumHullValid = false;
}

void Frustum::AdjustHfoviForAspectRatio(double aspectRatio)
{
	this->vfovi = FRUMPY_RADS_TO_DEGS(2.0 * atan(tan(FRUMPY_DEGS_TO_RADS(this->vfovi) / 2) * aspectRatio));
	this->frustumHullValid = false;
}

void Frustum::CalcProjectionMatrix(Matrix& projectionMatrix) const
{
	projectionMatrix.Projection(FRUMPY_DEGS_TO_RADS(this->hfovi), FRUMPY_DEGS_TO_RADS(this->vfovi), this->_near, this->_far);
}

const ConvexHull& Frustum::GetFrustumHull() const
{
	if (!this->frustumHullValid)
	{
		this->frustumHull.Generate(*this);
		this->frustumHullValid = true;
	}

	return this->frustumHull;
}