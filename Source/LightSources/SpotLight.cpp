#include "SpotLight.h"
#include "../Renderer.h"

using namespace Frumpy;

SpotLight::SpotLight()
{
	this->locationWorldSpace.SetComponents(0.0, 0.0, 0.0);
	this->directionWorldSpace.SetComponents(1.0, 0.0, 0.0);
	this->attenuationRadius = 10.0;
}

/*virtual*/ SpotLight::~SpotLight()
{
}

/*virtual*/ void SpotLight::PrepareForRender(const GraphicsMatrices& graphicsMatrices) const
{
	graphicsMatrices.worldToCamera.TransformPoint(this->locationWorldSpace, this->locationCameraSpace);
	graphicsMatrices.worldToCamera.TransformVector(this->directionWorldSpace, this->directionCameraSpace);
}

/*virtual*/ void SpotLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const
{
	// TODO: Write this stupid code.  It's all stupid.
}