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

Camera::Frustum::Frustum()
{
	this->_near = 0.1;
	this->_far = 1000.0;
	this->hfovi = 60.0;
	this->vfovi = 50.0;
}

void Camera::Frustum::AdjustVFoviForAspectRatio(double aspectRatio)
{
	this->vfovi = FRUMPY_RADS_TO_DEGS(2.0 * atan(tan(FRUMPY_DEGS_TO_RADS(this->hfovi) / 2.0) / aspectRatio));
}

void Camera::Frustum::AdjustHfoviForAspectRatio(double aspectRatio)
{
	this->vfovi = FRUMPY_RADS_TO_DEGS(2.0 * atan(tan(FRUMPY_DEGS_TO_RADS(this->vfovi) / 2) * aspectRatio));
}

void Camera::Frustum::GeneratePlanes(List<Plane>& frustumPlanesList) const
{
	double hfoviRads = FRUMPY_DEGS_TO_RADS(this->hfovi);
	double vfoviRads = FRUMPY_DEGS_TO_RADS(this->vfovi);

	frustumPlanesList.Clear();

	// Top/bottom planes:
	frustumPlanesList.AddTail(Plane(Vector(0.0, sin((vfoviRads + FRUMPY_PI) / 2.0), -cos((vfoviRads + FRUMPY_PI) / 2.0)), Vector(0.0, tan(vfoviRads / 2.0), -1.0)));
	frustumPlanesList.AddTail(Plane(Vector(0.0, -sin((vfoviRads + FRUMPY_PI) / 2.0), -cos((vfoviRads + FRUMPY_PI) / 2.0)), Vector(0.0, -tan(vfoviRads / 2.0), -1.0)));

	// Left/right planes:
	frustumPlanesList.AddTail(Plane(Vector(sin((hfoviRads + FRUMPY_PI) / 2.0), -cos((hfoviRads + FRUMPY_PI) / 2.0), 0.0), Vector(tan(hfoviRads / 2.0), 0.0, -1.0)));
	frustumPlanesList.AddTail(Plane(Vector(-sin((hfoviRads + FRUMPY_PI) / 2.0), -cos((hfoviRads + FRUMPY_PI) / 2.0), 0.0), Vector(-tan(hfoviRads / 2.0), 0.0, -1.0)));

	// Near/far planes:
	frustumPlanesList.AddTail(Plane(Vector(0.0, 0.0, -this->_near), Vector(0.0, 0.0, 1.0)));
	frustumPlanesList.AddTail(Plane(Vector(0.0, 0.0, -this->_far), Vector(0.0, 0.0, -1.0)));
}

void Camera::Frustum::CalcProjectionMatrix(Matrix& projectionMatrix) const
{
	projectionMatrix.Projection(FRUMPY_DEGS_TO_RADS(this->hfovi), FRUMPY_DEGS_TO_RADS(this->vfovi), this->_near, this->_far);
}