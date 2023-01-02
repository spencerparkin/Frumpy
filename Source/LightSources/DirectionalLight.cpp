#include "DirectionalLight.h"
#include "../Renderer.h"

using namespace Frumpy;

DirectionalLight::DirectionalLight()
{
	this->directionWorldSpace.SetComponents(0.0, -1.0, 0.0);
}

/*virtual*/ DirectionalLight::~DirectionalLight()
{
}

/*virtual*/ void DirectionalLight::PrepareForRender(const GraphicsMatrices& graphicsMatrices) const
{
	graphicsMatrices.worldToCamera.TransformVector(this->directionWorldSpace, this->directionCameraSpace);
}

/*virtual*/ void DirectionalLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const
{
	double dot = Vector::Dot(surfaceProperties.normal, this->directionCameraSpace);
	if (dot < 0.0)
		surfaceColor = surfaceProperties.diffuseColor * -dot * this->mainIntensity;

	surfaceColor += surfaceProperties.diffuseColor * this->ambientIntensity;

	surfaceColor.x = FRUMPY_CLAMP(surfaceColor.x, 0.0, 1.0);
	surfaceColor.y = FRUMPY_CLAMP(surfaceColor.y, 0.0, 1.0);
	surfaceColor.z = FRUMPY_CLAMP(surfaceColor.z, 0.0, 1.0);
}