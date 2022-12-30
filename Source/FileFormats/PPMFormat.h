#pragma once

#include "../AssetManager.h"
#include <stdio.h>
#include <vector>

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

		struct Token
		{
			std::vector<char> charArray;
		};

		void Tokenize(FILE* file, std::vector<Token>& tokenArray, std::vector<unsigned char>& imageRaster);
	};
}