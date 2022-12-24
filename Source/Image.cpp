#include "Image.h"
#include "Vector.h"

using namespace Frumpy;

Image::Image()
{
	this->pixelData = nullptr;
	this->width = 0;
	this->height = 0;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->pixelData = nullptr;
	this->SetWidthAndHeight(width, height);
}

/*virtual*/ Image::~Image()
{
	delete[] this->pixelData;
}

void Image::SetWidthAndHeight(unsigned int width, unsigned int height)
{
	if (width != this->width || height != this->height)
	{
		delete[] this->pixelData;
		this->pixelData = nullptr;
		this->width = width;
		this->height = height;

		if (this->width > 0 && this->height > 0)
		{
			unsigned int pixelDataSize = this->GetRawPixelBufferSize();
			this->pixelData = new Pixel[pixelDataSize];
		}
	}
}

void Image::Clear(const Pixel& pixel)
{
	for (unsigned int i = 0; i < this->GetRawPixelBufferSize(); i++)
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

Image::Pixel* Image::GetPixel(const Vector& texCoords)
{
	Location location;
	location.row = FRUMPY_CLAMP(unsigned int(texCoords.y * double(this->height - 1)), 0, this->height - 1);
	location.col = FRUMPY_CLAMP(unsigned int(texCoords.x * double(this->width - 1)), 0, this->width - 1);
	return this->GetPixel(location);
}

const Image::Pixel* Image::GetPixel(const Vector& texCoords) const
{
	return const_cast<Image*>(this)->GetPixel(texCoords);
}

void Image::CalcImageMatrix(Matrix& imageMatrix) const
{
	imageMatrix.Identity();
	imageMatrix.ele[0][0] = double(this->width) / 2.0;
	imageMatrix.ele[0][3] = double(this->width) / 2.0;
	imageMatrix.ele[1][1] = double(this->height) / 2.0;
	imageMatrix.ele[1][3] = double(this->height) / 2.0;
}