#include "DirectionalLight.h"
#include "Renderer.h"

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

/*virtual*/ void DirectionalLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector4& surfaceColor, const Image* shadowBuffer) const
{
	double dot = Vector3::Dot(surfaceProperties.cameraSpaceNormal, this->cameraSpaceDirection);
	if (dot < 0.0)
		surfaceColor = surfaceProperties.diffuseColor * -dot * this->mainIntensity;

	surfaceColor += surfaceProperties.diffuseColor * this->ambientIntensity;

	surfaceColor.r = FRUMPY_CLAMP(surfaceColor.r, 0.0, 1.0);
	surfaceColor.g = FRUMPY_CLAMP(surfaceColor.g, 0.0, 1.0);
	surfaceColor.b = FRUMPY_CLAMP(surfaceColor.b, 0.0, 1.0);
	surfaceColor.a = FRUMPY_CLAMP(surfaceColor.a, 0.0, 1.0);
}