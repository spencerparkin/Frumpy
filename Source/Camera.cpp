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

Camera::Frustum::Frustum()
{
	this->near = 0.1;
	this->far = 1000.0;
	this->hfovi = 60.0;
	this->vfovi = 50.0;
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
	frustumPlanesList.AddTail(Plane(Vector(0.0, 0.0, -this->near), Vector(0.0, 0.0, 1.0)));
	frustumPlanesList.AddTail(Plane(Vector(0.0, 0.0, -this->far), Vector(0.0, 0.0, -1.0)));
}

void Camera::Frustum::CalcProjectionMatrix(Matrix& projectionMatrix) const
{
	projectionMatrix.Projection(this->hfovi, this->vfovi, this->near, this->far);
}