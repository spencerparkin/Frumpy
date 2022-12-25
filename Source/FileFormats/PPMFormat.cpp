#include "PPMFormat.h"

using namespace Frumpy;

PPMFormat::PPMFormat()
{
}

/*virtual*/ PPMFormat::~PPMFormat()
{
}

/*virtual*/ const char* PPMFormat::SupportedExtension()
{
	return "PPM";
}

/*virtual*/ bool PPMFormat::LoadAssets(const char* filePath, List<Asset*>& assetList)
{
	// TODO: Write this.
	return true;
}

/*virtual*/ bool PPMFormat::SaveAssets(const char* filePath, const List<Asset*>& assetList)
{
	// TODO: Write this.
	return true;
}