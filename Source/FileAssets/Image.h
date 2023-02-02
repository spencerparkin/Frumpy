#pragma once

#include "Defines.h"
#include "Math/Matrix4x4.h"
#include "AssetManager.h"
#include <stdint.h>

namespace Frumpy
{
	class Vertex;

	// An image can be a render target or a render source (texture).
	// All images are of the same format: RGBA, 32-bit, and can be depth buffers too.
	// The goal of Frumpy is simply to render into one of these images.
	// It is up to the host application to get the image displayed on the screen,
	// or to do any kind of double-buffering.
	class FRUMPY_API Image : public AssetManager::Asset
	{
	public:
		Image();
		Image(unsigned int width, unsigned int height);
		virtual ~Image();

		virtual AssetManager::Asset* Clone() const override;

		struct Pixel;

		void SetWidthAndHeight(unsigned int width, unsigned int height);

		const unsigned char* GetRawPixelBuffer() const
		{
			return reinterpret_cast<unsigned char*>(pixelData);
		}

		Pixel* GetRawPixelBuffer()
		{
			return this->pixelData;
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

		void CalcImageMatrix(Matrix4x4& imageMatrix) const;

		void SetAsCopyOf(const Image* image);

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
				float depth;
			};

			void GetColorComponents(uint32_t& r, uint32_t& g, uint32_t& b, uint32_t& a, const Image* image) const;
			void MakeColorVector(Vector4& colorVector, const Image* image) const;
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

		bool SetPixel(const Location& location, uint32_t color);

		uint32_t MakeColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a) const;
		uint32_t MakeColor(const Vector4& colorVector) const;

		Pixel* GetPixel(unsigned int i)
		{
			return &this->pixelData[i];
		}

		enum SampleMethod
		{
			NEAREST,
			BILINEAR
		};

		void SampleColorVector(Vector4& colorVector, const Vector3& texCoords, SampleMethod sampleMethod) const;
		void SampleColorVector(Vector4& colorVector, const Location& location) const;

		float SampleDepth(double uCoord, double vCoord) const;

		void ConvertDepthToGreyScale(float depthIgnore);

	private:

		Pixel* pixelData;
		unsigned int width;
		unsigned int height;
		bool ownsMemory;
		Format format;
	};
}