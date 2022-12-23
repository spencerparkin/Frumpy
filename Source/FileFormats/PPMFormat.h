#pragma once

#include "../FileFormat.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace Frumpy
{
	class Mesh;

	class FRUMPY_API PPMFormat : public FileFormat
	{
	public:
		PPMFormat();
		virtual ~PPMFormat();

		virtual const char* SupportedExtension() override;

		virtual bool LoadAssets(const char* filePath, List<Asset*>& assetList) override;
		virtual bool SaveAssets(const char* filePath, const List<Asset*>& assetList) override;
	};
}