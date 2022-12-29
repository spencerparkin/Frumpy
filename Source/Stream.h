#pragma once

#include "Defines.h"

namespace Frumpy
{
	class FRUMPY_API Stream
	{
	public:
		Stream();
		virtual ~Stream();

		virtual bool Read(unsigned char* data, unsigned int size) = 0;
		virtual bool Write(const unsigned char* data, unsigned int size) = 0;
	};

	class FRUMPY_API BufferStream : public Stream
	{
	public:
		BufferStream();
		virtual ~BufferStream();

		virtual bool Read(unsigned char* data, unsigned int size) override;
		virtual bool Write(const unsigned char* data, unsigned int size) override;

		unsigned char* buffer;
		unsigned int bufferSize;
		unsigned int readOffset;
		unsigned int writeOffset;
		bool ownsMemory;
	};
}