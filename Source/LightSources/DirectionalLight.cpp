#include "DirectionalLight.h"
#include "../Renderer.h"

using namespace Frumpy;

DirectionalLight::DirectionalLight()
{
	this->worldSpaceDirection.SetComponents(0.0, -1.0, 0.0);
}

/*virtual*/ DirectionalLight::~DirectionalLight()
{
}

/*virtual*/ void DirectionalLight::PrepareForRender(const GraphicsMatrices& graphicsMatrices) const
{
	graphicsMatrices.worldToCamera.TransformVector(this->worldSpaceDirection, this->cameraSpaceDirection);
}

/*virtual*/ void DirectionalLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector3& surfaceColor, const Image* shadowBuffer) const
{
	double dot = Vector3::Dot(surfaceProperties.cameraSpaceNormal, this->cameraSpaceDirection);
	if (dot < 0.0)
		surfaceColor = surfaceProperties.diffuseColor * -dot * this->mainIntensity;

	surfaceColor += surfaceProperties.diffuseColor * this->ambientIntensity;

	surfaceColor.x = FRUMPY_CLAMP(surfaceColor.x, 0.0, 1.0);
	surfaceColor.y = FRUMPY_CLAMP(surfaceColor.y, 0.0, 1.0);
	surfaceColor.z = FRUMPY_CLAMP(surfaceColor.z, 0.0, 1.0);
}