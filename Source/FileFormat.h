#pragma once

#include "Defines.h"
#include "List.h"
#include <string>

namespace Frumpy
{
	class FRUMPY_API FileFormat
	{
	public:
		FileFormat();
		virtual ~FileFormat();

		// Derivatives of this are anything that can be saved/loaded to/from disk.
		class FRUMPY_API Asset
		{
		public:
			Asset();
			virtual ~Asset();

			virtual Asset* Clone() const;

			std::string* name;
		};

		virtual const char* SupportedExtension() = 0;

		virtual bool LoadAssets(const char* filePath, List<Asset*>& assetList) = 0;
		virtual bool SaveAssets(const char* filePath, const List<Asset*>& assetList) = 0;

		char errorMessage[512];
	};
}