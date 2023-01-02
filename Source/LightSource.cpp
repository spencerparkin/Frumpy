#include "LightSource.h"

using namespace Frumpy;

LightSource::LightSource()
{
	this->mainIntensity = 1.0;
	this->ambientIntensity = 0.0;
}

/*virtual*/ LightSource::~LightSource()
{
}

/*virtual*/ bool LightSource::CalcShadowCamera(Camera& shadowCamera) const
{
	return false;
}