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

/*virtual*/ bool PPMFormat::LoadAssets(const char* filePath, List<AssetManager::Asset*>& assetList)
{
	return true;
}

/*virtual*/ bool PPMFormat::SaveAssets(const char* filePath, const List<AssetManager::Asset*>& assetList)
{
	return true;
}