#include "Image.h"
#include "Vertex.h"

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

void Image::RenderTriangle(const Vertex& vertexA, const Vertex& vertexB, const Vertex& vertexC, const Matrix& transformMatrix, Image& depthBuffer)
{
	// TODO: This is where the rubber meets the road.  How do we do it?  Converting from projection space to image space is easy, and
	//       so is the process of filling in the triangle, one scan-line at a time.  The hard part is interpolating depth values and
	//       other things.
}