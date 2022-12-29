#pragma once

#include "Stream.h"

using namespace Frumpy;

Stream::Stream()
{
}

/*virtual*/ Stream::~Stream()
{
}

BufferStream::BufferStream()
{
	this->buffer = nullptr;
	this->bufferSize = 0;
	this->readOffset = 0;
	this->writeOffset = 0;
	this->ownsMemory = false;
}

/*virtual*/ BufferStream::~BufferStream()
{
	if (this->ownsMemory)
		delete[] this->buffer;
}

/*virtual*/ bool BufferStream::Read(unsigned char* data, unsigned int size)
{
	if (this->bufferSize - this->readOffset < size)
		return false;

	for (unsigned int i = 0; i < size; i++)
		data[i] = this->buffer[this->readOffset++];

	return true;
}

/*virtual*/ bool BufferStream::Write(const unsigned char* data, unsigned int size)
{
	if (this->bufferSize - this->readOffset < size)
		return false;

	for (unsigned int i = 0; i < size; i++)
		this->buffer[this->writeOffset++] = data[i];

	return true;
}