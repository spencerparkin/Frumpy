#pragma once

#include "../Defines.h"
#include "../Matrix.h"
#include "../FileFormat.h"
#include <stdint.h>

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

		void SetRawPixelBuffer(void* buffer, unsigned int width, unsigned int height);

		unsigned int GetNumPixels() const
		{
			return this->width * this->height;
		}

		unsigned int GetRawPixelBufferSize() const
		{
			return this->GetNumPixels() * sizeof(Pixel);
		}

		unsigned int GetWidth() const { return this->width; }
		unsigned int GetHeight() const { return this->height; }

		double GetAspectRatio() const { return double(this->width) / double(this->height); }

		void CalcImageMatrix(Matrix& imageMatrix) const;

		struct Format
		{
			unsigned int rShift, gShift, bShift, aShift;
		};

		void SetFormat(const Format& format) { this->format = format; }
		Format GetFormat() const { return this->format; }

		struct Pixel
		{
			union
			{
				uint32_t color;
				volatile float depth;
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

		Pixel* GetPixel(const Vector& texCoords);
		const Pixel* GetPixel(const Vector& texCoords) const;

		bool SetPixel(const Location& location, uint32_t color);
		uint32_t MakeColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a);

	private:

		Pixel* pixelData;
		unsigned int width;
		unsigned int height;
		bool ownsMemory;
		Format format;
	};
}