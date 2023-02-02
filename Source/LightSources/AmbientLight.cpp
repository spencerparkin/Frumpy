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

/*virtual*/ void AmbientLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector4& surfaceColor, const Image* shadowBuffer) const
{
	surfaceColor = surfaceProperties.diffuseColor * this->ambientIntensity;

	surfaceColor.r = FRUMPY_CLAMP(surfaceColor.r, 0.0, 1.0);
	surfaceColor.g = FRUMPY_CLAMP(surfaceColor.g, 0.0, 1.0);
	surfaceColor.b = FRUMPY_CLAMP(surfaceColor.b, 0.0, 1.0);
	surfaceColor.a = FRUMPY_CLAMP(surfaceColor.a, 0.0, 1.0);
}