#include "PPMFormat.h"
#include "FileAssets/Image.h"
#include <stdio.h>

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
	Image* image = nullptr;

	std::vector<Token> tokenArray;
	std::vector<unsigned char> imageRaster;
	FILE* file = nullptr;
	fopen_s(&file, filePath, "rb");
	if (file)
	{
		this->Tokenize(file, tokenArray, imageRaster);
		fclose(file);
	}

	if (tokenArray.size() >= 4)
	{
		const char* ppmType = tokenArray[0].charArray.data();
		unsigned int width = atoi(tokenArray[1].charArray.data());
		unsigned int height = atoi(tokenArray[2].charArray.data());
		unsigned int maxVal = atoi(tokenArray[3].charArray.data());

		size_t imageRasterSize = imageRaster.size();
		if (0 == strcmp(ppmType, "P6") && imageRasterSize == width * height * 3 && maxVal == 255)
		{
			image = new Image(width, height);
			const unsigned char* pixelData = imageRaster.data();
			for (unsigned int row = 0; row < height; row++)
			{
				for (unsigned int col = 0; col < width; col++)
				{
					// Our internal image format has (0,0) in the lower-left corner.
					// Most external image foramts have (0,0) in the upper-left corner.
					Image::Pixel* dstPixel = image->GetPixel(Image::Location{ height - 1 - row, col });
					const unsigned char* srcPixel = &pixelData[row * width * 3 + col * 3];
					dstPixel->color = image->MakeColor(srcPixel[0], srcPixel[1], srcPixel[2], 0);
				}
			}
		}
		else if (0 == strcmp(ppmType, "P3") && maxVal == 255)
		{
			image = new Image(width, height);
			unsigned int i = 4;
			for (unsigned int row = 0; row < height && i < tokenArray.size() - 3; row++)
			{
				for (unsigned int col = 0; col < width && i < tokenArray.size() - 3; col++)
				{
					Image::Pixel* dstPixel = image->GetPixel(Image::Location{ height - 1 - row, col });
					unsigned int r = atoi(tokenArray[i + 0].charArray.data());
					unsigned int g = atoi(tokenArray[i + 1].charArray.data());
					unsigned int b = atoi(tokenArray[i + 2].charArray.data());
					dstPixel->color = image->MakeColor(r, g, b, 0);
					i += 3;
				}
			}
		}
	}

	if (image)
	{
		strcpy_s(image->name, sizeof(image->name), filePath);
		assetList.AddTail(image);
	}

	return image ? true : false;
}

void PPMFormat::Tokenize(FILE* file, std::vector<Token>& tokenArray, std::vector<unsigned char>& imageRaster)
{
	tokenArray.clear();
	imageRaster.clear();

	Token token;
	bool inComment = false;

	while (true)
	{
		char byte = 0;
		size_t bytesRead = fread(&byte, 1, 1, file);
		if (bytesRead == 0)
			break;

		if (!iswspace(byte))
		{
			if (byte == '#')
				inComment = true;

			if (!inComment)
				token.charArray.push_back(byte);
		}
		else
		{
			if (token.charArray.size() > 0)
			{
				token.charArray.push_back('\0');
				tokenArray.push_back(token);
				token.charArray.clear();

				if (tokenArray.size() == 4 && 0 == strcmp("P6", tokenArray[0].charArray.data()))
				{
					long curPos = ftell(file);
					fseek(file, 0, SEEK_END);
					long endPos = ftell(file);
					long size = endPos - curPos;
					imageRaster.resize(size);
					fseek(file, curPos, SEEK_SET);
					bytesRead = fread(imageRaster.data(), 1, size, file);
					break;
				}
			}
			
			if (byte == '\n')
				inComment = false;
		}
	}

	if (token.charArray.size() > 0)
	{
		token.charArray.push_back('\0');
		tokenArray.push_back(token);
	}
}

/*virtual*/ bool PPMFormat::SaveAssets(const char* filePath, const List<const AssetManager::Asset*>& assetList)
{
	if (assetList.GetCount() != 1)
		return false;

	const Image* image = dynamic_cast<const Image*>(assetList.GetHead()->value);
	if (!image)
		return false;

	FILE* file = nullptr;
	fopen_s(&file, filePath, "w");
	if (!file)
		return false;

	fprintf(file, "P3\n");
	fprintf(file, "# Generated by Frumpy\n");
	fprintf(file, "%d %d\n", image->GetWidth(), image->GetHeight());
	fprintf(file, "255\n");

	for (unsigned int i = 0; i < image->GetHeight(); i++)
	{
		for (unsigned int j = 0; j < image->GetWidth(); j++)
		{
			const Image::Pixel* pixel = image->GetPixel(Image::Location{ image->GetHeight() - 1 - i, j });
			uint32_t r, g, b, a;
			pixel->GetColorComponents(r, g, b, a, image);
			fprintf(file, "%03d %03d %03d\t", r, g, b);		// TODO: Alpha support?
		}

		fprintf(file, "\n");
	}

	fclose(file);
	return true;
}