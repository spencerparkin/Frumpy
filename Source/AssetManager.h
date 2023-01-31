#pragma once

#include "Defines.h"
#include "Map.h"

namespace Frumpy
{
	class FRUMPY_API AssetManager
	{
	public:
		AssetManager();
		virtual ~AssetManager();

		class FRUMPY_API Asset
		{
		public:
			Asset();
			virtual ~Asset();

			virtual Asset* Clone() const;

			char name[256];
		};

		bool LoadAssets(const char* filePath);
		bool SaveAsset(const char* filePath, const Asset* asset);

		Asset* FindAssetByName(const char* assetName);

		bool AddAsset(Asset* asset);

		class FRUMPY_API FileFormat
		{
		public:
			FileFormat();
			virtual ~FileFormat();

			virtual const char* SupportedExtension() = 0;

			virtual bool LoadAssets(const char* filePath, List<Asset*>& assetList) = 0;
			virtual bool SaveAssets(const char* filePath, const List<const Asset*>& assetList) = 0;
		};

	private:

		bool GetExtension(const char* filePath, char* extension, unsigned int extensionSize);

		FileFormat* FindApplicableFileFormat(const char* filePath);

		typedef Map<Asset*> AssetMap;
		AssetMap assetMap;

		typedef List<FileFormat*> FileFormatList;
		FileFormatList fileFormatList;
	};
}