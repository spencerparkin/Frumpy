#include "Image.h"
#include "Math/Vector3.h"
#include <string.h>
#include <math.h>

using namespace Frumpy;

Image::Image()
{
	this->ownsMemory = false;
	this->pixelData = nullptr;
	this->width = 0;
	this->height = 0;
	this->format.rShift = 0;
	this->format.gShift = 8;
	this->format.bShift = 16;
	this->format.aShift = 24;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->ownsMemory = false;
	this->pixelData = nullptr;
	this->format.rShift = 0;
	this->format.gShift = 8;
	this->format.bShift = 16;
	this->format.aShift = 24;
	this->SetWidthAndHeight(width, height);
}

/*virtual*/ Image::~Image()
{
	if (this->ownsMemory)
		delete[] this->pixelData;
}

void Image::SetRawPixelBuffer(void* buffer, unsigned int width, unsigned int height)
{
	if (this->ownsMemory)
		delete[] this->pixelData;

	this->pixelData = (Pixel*)buffer;
	this->ownsMemory = false;
	this->width = width;
	this->height = height;
}

bool Image::SetPixel(const Location& location, uint32_t color)
{
	Pixel* pixel = this->GetPixel(location);
	if (!pixel)
		return false;

	pixel->color = color;
	return true;
}

uint32_t Image::MakeColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a) const
{
	return (r << this->format.rShift) | (g << this->format.gShift) | (b << this->format.bShift) | (a << this->format.aShift);
}

uint32_t Image::MakeColor(const Vector3& colorVector) const
{
	unsigned int r = FRUMPY_CLAMP((unsigned int)(colorVector.x * 255.0), 0, 255);
	unsigned int g = FRUMPY_CLAMP((unsigned int)(colorVector.y * 255.0), 0, 255);
	unsigned int b = FRUMPY_CLAMP((unsigned int)(colorVector.z * 255.0), 0, 255);

	return this->MakeColor(r, g, b, 0);
}

void Image::SetWidthAndHeight(unsigned int width, unsigned int height)
{
	if (width != this->width || height != this->height)
	{
		if (this->ownsMemory)
			delete[] this->pixelData;
		this->pixelData = nullptr;
		this->width = width;
		this->height = height;

		if (this->width > 0 && this->height > 0)
		{
			unsigned int pixelDataSize = this->GetNumPixels();
			this->pixelData = new Pixel[pixelDataSize];
			this->ownsMemory = true;
		}
	}
}

/*virtual*/ AssetManager::Asset* Image::Clone() const
{
	Image* image = new Image();
	image->SetAsCopyOf(this);
	return image;
}

void Image::SetAsCopyOf(const Image* image)
{
	this->SetWidthAndHeight(image->GetWidth(), image->GetHeight());
	memcpy(this->pixelData, image->pixelData, this->GetRawPixelBufferSize());
	this->format = image->format;
}

void Image::Clear(const Pixel& pixel)
{
	for (unsigned int i = 0; i < this->GetNumPixels(); i++)
		this->pixelData[i] = pixel;
}

bool Image::ValidLocation(const Location& location) const
{
	if (location.row >= this->height)
		return false;

	if (location.col >= this->width)
		return false;

	return true;
}

Image::Pixel* Image::GetPixel(const Location& location)
{
	// TODO: Maybe compile this check out of release.
	if (!this->ValidLocation(location))
		return nullptr;

	return &this->pixelData[location.row * this->width + location.col];
}

const Image::Pixel* Image::GetPixel(const Location& location) const
{
	return const_cast<Image*>(this)->GetPixel(location);
}

float Image::SampleDepth(double uCoord, double vCoord) const
{
	double approxRow = vCoord * double(this->height - 1);
	double approxCol = uCoord * double(this->width - 1);

	Location location;
	location.row = FRUMPY_CLAMP((unsigned int)(approxRow), 0, this->height - 1);
	location.col = FRUMPY_CLAMP((unsigned int)(approxCol), 0, this->width - 1);

	const Pixel* pixel = this->GetPixel(location);
	return pixel->depth;
}

void Image::SampleColorVector(Vector3& colorVector, const Vector3& texCoords, SampleMethod sampleMethod) const
{
	double approxRow = texCoords.y * double(this->height);
	double approxCol = texCoords.x * double(this->width);

	switch (sampleMethod)
	{
		case NEAREST:
		{
			unsigned int row = unsigned int(approxRow) % this->height;
			unsigned int col = unsigned int(approxCol) % this->width;
			this->SampleColorVector(colorVector, Location{ row, col });
			break;
		}
		case BILINEAR:
		{
			double minRow = floor(approxRow);
			double maxRow = minRow + 1.0;
			double minCol = floor(approxCol);
			double maxCol = minCol + 1.0;

			double area00 = (approxRow - minRow) * (approxCol - minCol);
			double area01 = (approxRow - minRow) * (maxCol - approxCol);
			double area10 = (maxRow - approxRow) * (approxCol - minCol);
			double area11 = (maxRow - approxRow) * (maxCol - approxCol);

			unsigned int minRowInt = unsigned int(minRow) % this->height;
			unsigned int maxRowInt = unsigned int(maxRow) % this->height;
			unsigned int minColInt = unsigned int(minCol) % this->width;
			unsigned int maxColInt = unsigned int(maxCol) % this->width;

			Vector3 color00, color01, color10, color11;

			this->SampleColorVector(color00, Location{ minRowInt, minColInt });
			this->SampleColorVector(color01, Location{ minRowInt, maxColInt });
			this->SampleColorVector(color10, Location{ maxRowInt, minColInt });
			this->SampleColorVector(color11, Location{ maxRowInt, maxColInt });

			colorVector =
				color00 * area11 +
				color01 * area10 +
				color10 * area01 +
				color11 * area00;

			break;
		}
		default:
		{
			colorVector.SetComponents(1.0, 1.0, 1.0);
			break;
		}
	}
}

void Image::SampleColorVector(Vector3& colorVector, const Location& location) const
{
	const Pixel* pixel = this->GetPixel(location);

	colorVector.x = double((pixel->color & (0xFF << this->format.rShift)) >> this->format.rShift) / 255.0;
	colorVector.y = double((pixel->color & (0xFF << this->format.gShift)) >> this->format.gShift) / 255.0;
	colorVector.z = double((pixel->color & (0xFF << this->format.bShift)) >> this->format.bShift) / 255.0;
}

void Image::CalcImageMatrix(Matrix4x4& imageMatrix) const
{
	imageMatrix.Identity();
	imageMatrix.ele[0][0] = double(this->width) / 2.0;
	imageMatrix.ele[0][3] = double(this->width) / 2.0;
	imageMatrix.ele[1][1] = double(this->height) / 2.0;
	imageMatrix.ele[1][3] = double(this->height) / 2.0;
}

void Image::Pixel::GetColorComponents(uint32_t& r, uint32_t& g, uint32_t& b, const Image* image) const
{
	r = (this->color & (0xFF << image->format.rShift)) >> image->format.rShift;
	g = (this->color & (0xFF << image->format.gShift)) >> image->format.gShift;
	b = (this->color & (0xFF << image->format.bShift)) >> image->format.bShift;
}

void Image::Pixel::MakeColorVector(Vector3& colorVector, const Image* image) const
{
	uint32_t r, g, b;

	this->GetColorComponents(r, g, b, image);

	colorVector.x = double(r) / 255.0;
	colorVector.y = double(g) / 255.0;
	colorVector.z = double(b) / 255.0;
}

void Image::ConvertDepthToGreyScale(float depthIgnore)
{
	float minZ = 9999.0;
	float maxZ = -9999.0;

	for (unsigned int i = 0; i < this->height; i++)
	{
		for (unsigned int j = 0; j < this->width; j++)
		{
			Pixel* pixel = this->GetPixel(Location{ i, j });
			float depth = -pixel->depth;

			if (depth != depthIgnore)
			{
				if (minZ > depth)
					minZ = depth;
				if (maxZ < depth)
					maxZ = depth;
			}
		}
	}

	for (unsigned int i = 0; i < this->height; i++)
	{
		for (unsigned int j = 0; j < this->width; j++)
		{
			Pixel* pixel = this->GetPixel(Location{ i, j });
			float depth = -pixel->depth;
			if (depth == depthIgnore)
			{
				Vector3 colorVector(1.0, 0.0, 0.0);
				pixel->color = this->MakeColor(colorVector);
			}
			else
			{
				float scale = (depth - minZ) / (maxZ - minZ);
				scale = FRUMPY_CLAMP(scale, 0.0f, 1.0f);
				Vector3 colorVector(scale, scale, scale);
				pixel->color = this->MakeColor(colorVector);
			}
		}
	}
}