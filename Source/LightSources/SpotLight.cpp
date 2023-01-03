#include "SpotLight.h"
#include "../Renderer.h"
#include <math.h>

using namespace Frumpy;

SpotLight::SpotLight()
{
	this->worldSpaceLocation.SetComponents(0.0, 0.0, 0.0);
	this->worldSpaceDirection.SetComponents(1.0, 0.0, 0.0);
	this->innerConeAngle = 10.0;
	this->outerConeAngle = 15.0;
}

/*virtual*/ SpotLight::~SpotLight()
{
}

/*virtual*/ void SpotLight::PrepareForRender(const GraphicsMatrices& graphicsMatrices) const
{
	graphicsMatrices.worldToCamera.TransformPoint(this->worldSpaceLocation, this->cameraSpaceLocation);
	graphicsMatrices.worldToCamera.TransformVector(this->worldSpaceDirection, this->cameraSpaceDirection);
}

/*virtual*/ void SpotLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor) const
{
	surfaceColor = surfaceProperties.diffuseColor * this->ambientIntensity;

	double dot = Vector::Dot(surfaceProperties.cameraSpaceNormal, this->cameraSpaceDirection);
	if (dot < 0.0)
	{
		Vector vector = surfaceProperties.cameraSpacePoint - this->cameraSpaceLocation;
		double length = vector.Length();	// TODO: This would factor in with the attenuation radius.  Use inverse square fall-off?
		double angleDegs = FRUMPY_RADS_TO_DEGS(acos(Vector::Dot(vector / length, this->cameraSpaceDirection)));

		double spotLightFactor = 0.0;
		if (angleDegs <= this->innerConeAngle)
			spotLightFactor = 1.0;
		else if (angleDegs <= this->outerConeAngle)
			spotLightFactor = 1.0 - (angleDegs - innerConeAngle) / (outerConeAngle - innerConeAngle);

		surfaceColor += surfaceProperties.diffuseColor * -dot * this->mainIntensity * spotLightFactor;
	}

	surfaceColor.x = FRUMPY_CLAMP(surfaceColor.x, 0.0, 1.0);
	surfaceColor.y = FRUMPY_CLAMP(surfaceColor.y, 0.0, 1.0);
	surfaceColor.z = FRUMPY_CLAMP(surfaceColor.z, 0.0, 1.0);
}