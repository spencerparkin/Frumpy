#include "Image.h"
#include "../Vector.h"
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

uint32_t Image::MakeColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a)
{
	return (r << this->format.rShift) | (g << this->format.gShift) | (b << this->format.bShift) | (a << this->format.aShift);
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
	if (!this->ValidLocation(location))
		return nullptr;

	return &this->pixelData[location.row * this->width + location.col];
}

const Image::Pixel* Image::GetPixel(const Location& location) const
{
	return const_cast<Image*>(this)->GetPixel(location);
}

void Image::SampleColorVector(Vector& colorVector, const Vector& texCoords) const
{
	double approxRow = texCoords.y * double(this->height - 1);
	double approxCol = texCoords.x * double(this->width - 1);
#if 0	// TODO: What's wrong with this code?  Or is it a problem with the given texture coordinates?
	double minRow = floor(approxRow);
	double maxRow = minRow + 1.0;
	double minCol = floor(approxCol);
	double maxCol = minCol + 1.0;

	double area00 = (approxRow - minRow) * (approxCol - minCol);
	double area01 = (approxRow - minRow) * (maxCol - approxCol);
	double area10 = (maxRow - approxRow) * (approxCol - minCol);
	double area11 = (maxRow - approxRow) * (maxCol - approxCol);

	unsigned int minRowInt = FRUMPY_CLAMP(unsigned int(minRow), 0, this->height - 1);
	unsigned int maxRowInt = FRUMPY_CLAMP(unsigned int(maxRow), 0, this->height - 1);
	unsigned int minColInt = FRUMPY_CLAMP(unsigned int(minCol), 0, this->width - 1);
	unsigned int maxColInt = FRUMPY_CLAMP(unsigned int(maxCol), 0, this->width - 1);

	Vector color00, color01, color10, color11;

	this->SampleColorVector(color00, Location{ minRowInt, minColInt });
	this->SampleColorVector(color01, Location{ minRowInt, maxColInt });
	this->SampleColorVector(color10, Location{ maxRowInt, minColInt });
	this->SampleColorVector(color11, Location{ maxRowInt, maxColInt });

	colorVector =
		color00 * area00 +
		color01 * area01 +
		color10 * area10 +
		color11 * area11;
#else
	Location location;
	location.row = FRUMPY_CLAMP(unsigned int(approxRow), 0, this->height - 1);
	location.col = FRUMPY_CLAMP(unsigned int(approxCol), 0, this->width - 1);
	this->SampleColorVector(colorVector, location);
#endif
}

void Image::SampleColorVector(Vector& colorVector, const Location& location) const
{
	const Pixel* pixel = this->GetPixel(location);

	colorVector.x = double((pixel->color & (0xFF << this->format.rShift)) >> this->format.rShift) / 255.0;
	colorVector.y = double((pixel->color & (0xFF << this->format.gShift)) >> this->format.gShift) / 255.0;
	colorVector.z = double((pixel->color & (0xFF << this->format.bShift)) >> this->format.bShift) / 255.0;
}

void Image::CalcImageMatrix(Matrix& imageMatrix) const
{
	imageMatrix.Identity();
	imageMatrix.ele[0][0] = double(this->width) / 2.0;
	imageMatrix.ele[0][3] = double(this->width) / 2.0;
	imageMatrix.ele[1][1] = double(this->height) / 2.0;
	imageMatrix.ele[1][3] = double(this->height) / 2.0;
}

void Image::Pixel::MakeColorVector(Vector& colorVector, const Image* image) const
{
	unsigned int r = (this->color & (0xFF << image->format.rShift)) >> image->format.rShift;
	unsigned int g = (this->color & (0xFF << image->format.gShift)) >> image->format.gShift;
	unsigned int b = (this->color & (0xFF << image->format.bShift)) >> image->format.bShift;

	colorVector.x = double(r) / 255.0;
	colorVector.y = double(g) / 255.0;
	colorVector.z = double(b) / 255.0;
}