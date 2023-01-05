#include "SpotLight.h"
#include "../Renderer.h"
#include "../Camera.h"
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

/*virtual*/ void SpotLight::CalcSurfaceColor(const SurfaceProperties& surfaceProperties, Vector& surfaceColor, const Image* shadowBuffer) const
{
	surfaceColor = surfaceProperties.diffuseColor * this->ambientIntensity;

	double dot = Vector::Dot(surfaceProperties.cameraSpaceNormal, this->cameraSpaceDirection);
	if (dot < 0.0)
	{
		Vector vectorFromLightSource = surfaceProperties.cameraSpacePoint - this->cameraSpaceLocation;
		double distanceToLightSource = vectorFromLightSource.Length();	// TODO: This would factor in with the attenuation radius.  Use inverse square fall-off?
		double angleDegs = FRUMPY_RADS_TO_DEGS(acos(Vector::Dot(vectorFromLightSource / distanceToLightSource, this->cameraSpaceDirection)));

		double spotLightFactor = 0.0;
		if (angleDegs <= this->innerConeAngle)
			spotLightFactor = 1.0;
		else if (angleDegs <= this->outerConeAngle)
			spotLightFactor = 1.0 - (angleDegs - innerConeAngle) / (outerConeAngle - innerConeAngle);

		double shadowFactor = 1.0;
		if (shadowBuffer && angleDegs <= this->outerConeAngle)
		{
			double shortestLengthToLightSource = 9999999.0;	// TODO: This comes from the shadow-buffer.
			if (distanceToLightSource > shortestLengthToLightSource)
			{
				// Our view of the light is obscurred.  We are in shadow!
				shadowFactor = 0.0;
			}
		}

		surfaceColor += surfaceProperties.diffuseColor * -dot * this->mainIntensity * spotLightFactor * shadowFactor;
	}

	surfaceColor.x = FRUMPY_CLAMP(surfaceColor.x, 0.0, 1.0);
	surfaceColor.y = FRUMPY_CLAMP(surfaceColor.y, 0.0, 1.0);
	surfaceColor.z = FRUMPY_CLAMP(surfaceColor.z, 0.0, 1.0);
}

/*virtual*/ bool SpotLight::CalcShadowCamera(Camera& shadowCamera) const
{
	if (!shadowCamera.LookAt(this->worldSpaceLocation, this->worldSpaceLocation + this->worldSpaceDirection, Vector(0.0, 1.0, 0.0)))
		return false;

	shadowCamera.frustum._near = 0.1;
	shadowCamera.frustum._far = 10000.0;
	shadowCamera.frustum.hfovi = 2.0 * this->outerConeAngle;
	shadowCamera.frustum.vfovi = 2.0 * this->outerConeAngle;

	return true;
}