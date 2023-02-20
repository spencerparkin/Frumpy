#include "AssetManager.h"
#include "FileFormats/OBJFormat.h"
#include "FileFormats/PPMFormat.h"
#include "FileFormats/FBXFormat.h"

using namespace Frumpy;

//----------------------- AssetManager -----------------------

AssetManager::AssetManager()
{
	this->fileFormatList.AddTail(new OBJFormat());
	this->fileFormatList.AddTail(new PPMFormat());
	this->fileFormatList.AddTail(new FBXFormat());
}

/*virtual*/ AssetManager::~AssetManager()
{
	this->assetMap.Delete();
	this->fileFormatList.Delete();
}

bool AssetManager::AddAsset(Asset* asset)
{
	Asset* existingAsset = nullptr;

	this->assetMap.Find(asset->name, existingAsset);
	if (existingAsset)
		delete existingAsset;

	this->assetMap.Insert(asset->name, asset);
	return true;
}

bool AssetManager::LoadAssets(const char* filePath)
{
	FileFormat* fileFormat = this->FindApplicableFileFormat(filePath);
	if (!fileFormat)
		return false;

	List<Asset*> assetList;
	if (!fileFormat->LoadAssets(filePath, assetList))
		return false;

	for (List<Asset*>::Node* node = assetList.GetHead(); node; node = node->GetNext())
		this->AddAsset(node->value);

	return true;
}

bool AssetManager::SaveAsset(const char* filePath, const Asset* asset)
{
	FileFormat* fileFormat = this->FindApplicableFileFormat(filePath);
	if (!fileFormat)
		return false;

	List<const Asset*> assetList;
	assetList.AddTail(asset);

	return fileFormat->SaveAssets(filePath, assetList);
}

AssetManager::FileFormat* AssetManager::FindApplicableFileFormat(const char* filePath)
{
	char extension[16];
	if (this->GetExtension(filePath, extension, sizeof(extension)))
	{
		for (FileFormatList::Node* node = this->fileFormatList.GetHead(); node; node = node->GetNext())
		{
			FileFormat* fileFormat = node->value;
			if (0 == strcmp(fileFormat->SupportedExtension(), extension))
				return fileFormat;
		}
	}
	
	return nullptr;
}

AssetManager::Asset* AssetManager::FindAssetByName(const char* assetName)
{
	Asset* foundAsset = nullptr;
	this->assetMap.Find(assetName, foundAsset);
	return foundAsset;
}

bool AssetManager::GetExtension(const char* filePath, char* extension, unsigned int extensionSize)
{
	if (extensionSize == 0)
		return false;

	extension[0] = '\0';

	bool copyExtension = false;
	unsigned int j = 0;
	for (unsigned int i = 0; filePath[i] != '\0'; i++)
	{
		if (filePath[i] == '.')
			copyExtension = true;
		else if (copyExtension)
		{
			if (j < extensionSize)
				extension[j++] = toupper(filePath[i]);
			else
				return false;
		}
	}

	if (j < extensionSize)
		extension[j] = '\0';
	else
		return false;

	return true;
}

//----------------------- AssetManager::Asset -----------------------

AssetManager::Asset::Asset()
{
	this->name[0] = '\0';
}

/*virtual*/ AssetManager::Asset::~Asset()
{
}

/*virtual*/ AssetManager::Asset* AssetManager::Asset::Clone() const
{
	return nullptr;
}

//----------------------- AssetManager::FileFormat -----------------------

AssetManager::FileFormat::FileFormat()
{
}

/*virtual*/ AssetManager::FileFormat::~FileFormat()
{
}