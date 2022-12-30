#pragma once

#include "../AssetManager.h"

namespace Frumpy
{
	class FRUMPY_API PPMFormat : public AssetManager::FileFormat
	{
	public:
		PPMFormat();
		virtual ~PPMFormat();

		virtual const char* SupportedExtension() override;

		virtual bool LoadAssets(const char* filePath, List<AssetManager::Asset*>& assetList) override;
		virtual bool SaveAssets(const char* filePath, const List<AssetManager::Asset*>& assetList) override;
	};
}