#include "AmbientLight.h"

using namespace Frumpy;

AmbientLight::AmbientLight()
{
	this->ambientIntensity = 1.0;
}

/*virtual*/ AmbientLight::~AmbientLight()
{
}

/*virtual*/ void AmbientLight::PrepareForRender(const GraphicsMatrices& graphicsMatrices) const
{
}

/*virtual*/ void AmbientLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector3& surfaceColor, const Image* shadowBuffer) const
{
	surfaceColor = surfaceProperties.diffuseColor * this->ambientIntensity;

	surfaceColor.x = FRUMPY_CLAMP(surfaceColor.x, 0.0, 1.0);
	surfaceColor.y = FRUMPY_CLAMP(surfaceColor.y, 0.0, 1.0);
	surfaceColor.z = FRUMPY_CLAMP(surfaceColor.z, 0.0, 1.0);
}