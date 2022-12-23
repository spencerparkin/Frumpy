#pragma once

#include "Defines.h"
#include "Matrix.h"
#include "FileFormat.h"

namespace Frumpy
{
	class Vertex;

	// An image can be a render target or a render source (texture).
	// All images are of the same format: RGBA, 32-bit, and can be depth buffers too.
	// The goal of Frumpy is simply to render into one of these images.
	// It is up to the host application to get the image displayed on the screen,
	// or to do any kind of double-buffering.
	class FRUMPY_API Image : public FileFormat::Asset
	{
	public:
		Image();
		Image(unsigned int width, unsigned int height);
		virtual ~Image();

		void SetWidthAndHeight(unsigned int width, unsigned int height);

		const unsigned char* GetRawPixelBuffer() const
		{
			return reinterpret_cast<unsigned char*>(pixelData);
		}

		unsigned int GetRawPixelBufferSize() const
		{
			return this->width * this->height * sizeof(Pixel);
		}

		unsigned int GetWidth() const { return this->width; }
		unsigned int GetHeight() const { return this->height; }

		double GetAspectRatio() const { return double(this->width) / double(this->height); }

		void CalcImageMatrix(Matrix& imageMatrix) const;

		struct Color
		{
			unsigned char red, green, blue, alpha;

			void SetColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
			{
				this->red = red;
				this->green = green;
				this->blue = blue;
				this->alpha = alpha;
			}
		};

		struct Pixel
		{
			union
			{
				Color color;
				float depth;
			};
		};

		void Clear(const Pixel& pixel);

		struct Location
		{
			unsigned int row;
			unsigned int col;
		};

		bool ValidLocation(const Location& location) const;

		Pixel* GetPixel(const Location& location);
		const Pixel* GetPixel(const Location& location) const;
		
	private:

		Pixel* pixelData;
		unsigned int width;
		unsigned int height;
	};
}