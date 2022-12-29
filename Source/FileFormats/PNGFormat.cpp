#include "PNGFormat.h"
#include "../FileAssets/Image.h"
#include "../Stream.h"
#include "zlib.h"
#include <string.h>

using namespace Frumpy;

PNGFormat::PNGFormat()
{
}

/*virtual*/ PNGFormat::~PNGFormat()
{
	this->chunkList.Delete();
}

/*virtual*/ const char* PNGFormat::SupportedExtension()
{
	return "PNG";
}

/*virtual*/ bool PNGFormat::LoadAssets(const char* filePath, List<Asset*>& assetList)
{
	this->chunkList.Delete();

	FILE* file = nullptr;
	fopen_s(&file, filePath, "rb");
	if (!file)
	{
		sprintf_s(this->errorMessage, sizeof(this->errorMessage), "Failed to open file: %s", filePath);
		return false;
	}

	fseek(file, 0, SEEK_END);
	unsigned int fileDataSize = (unsigned int)ftell(file);
	fseek(file, 0, SEEK_SET);
	unsigned char* fileData = new unsigned char[fileDataSize];
	fread(fileData, fileDataSize, 1, file);
	fclose(file);

	BufferStream stream;
	stream.buffer = fileData;
	stream.bufferSize = fileDataSize;
	stream.ownsMemory = true;

	unsigned char checkByte = 0;
	stream.Read(&checkByte, 1);
	if ((checkByte & 0x80) == 0)
	{
		sprintf_s(this->errorMessage, sizeof(this->errorMessage), "Check byte wrong.  Expected high bit set, got 0x%x.", checkByte);
		return false;
	}

	char png[4];
	stream.Read((unsigned char*)png, 3);
	png[3] = '\0';
	if (0 != strcmp(png, "PNG"))
	{
		sprintf_s(this->errorMessage, sizeof(this->errorMessage), "Did not recognize file as PNG file.");
		return false;
	}

	int skip = 0;
	stream.Read((unsigned char*)&skip, 4);

	while (true)
	{
		bool isCritical = false;
		Chunk* chunk = this->MakeChunkForChunkType(stream, isCritical);
		if (!chunk)
			break;

		if (!chunk->ReadData(stream) && isCritical)
		{
			sprintf_s(this->errorMessage, sizeof(this->errorMessage), "Failed to read critical chunk.");
			return false;
		}

		this->chunkList.AddTail(chunk);
	}

	HeaderChunk* headerChunk = this->FindChunkByType<HeaderChunk>();
	if (!headerChunk)
	{
		sprintf_s(this->errorMessage, sizeof(this->errorMessage), "No header chunk found!");
		return false;
	}

	if (headerChunk->width == 0 || headerChunk->height == 0)
	{
		sprintf_s(this->errorMessage, sizeof(this->errorMessage), "Can't handle image of unusual size %d x %d.", headerChunk->width, headerChunk->height);
		return false;
	}

	List<ImageDataChunk*> imageDataChunkList;
	this->GatherAllChunksOfType(imageDataChunkList);
	if (imageDataChunkList.GetCount() == 0)
	{
		sprintf_s(this->errorMessage, sizeof(this->errorMessage), "No image chunks found.");
		return false;
	}

	z_stream zStream;
	zStream.zalloc = Z_NULL;
	zStream.zfree = Z_NULL;
	zStream.opaque = Z_NULL;
	zStream.avail_in = 0;
	zStream.next_in = Z_NULL;
	int result = inflateInit(&zStream);
	if (result != Z_OK)
	{
		sprintf_s(this->errorMessage, sizeof(this->errorMessage), "InflateInit returned error code: %d", result);
		return false;
	}

	unsigned int filteredDataSize = headerChunk->width * headerChunk->height * 4 + headerChunk->height;		// Account for filter type bytes too.
	unsigned char* filteredData = new unsigned char[filteredDataSize];
	memset(filteredData, 0, filteredDataSize);
	List<ImageDataChunk*>::Node* node = imageDataChunkList.GetHead();
	zStream.next_in = node->value->buffer;
	zStream.avail_in = node->value->size;
	zStream.next_out = filteredData;
	zStream.avail_out = filteredDataSize;

	while (true)
	{
		if (zStream.avail_in == 0)
		{
			node = node->GetNext();
			if (node)
			{
				zStream.next_in = node->value->buffer;
				zStream.avail_in = node->value->size;
			}
		}

		result = inflate(&zStream, Z_NO_FLUSH);
		if (result == Z_STREAM_END)
		{
			result = inflate(&zStream, Z_FULL_FLUSH);
			break;
		}
		else if (result != Z_OK)
		{
			sprintf_s(this->errorMessage, sizeof(this->errorMessage), "Inflate returned error code: %d", result);
			break;
		}
	}

	inflateEnd(&zStream);

	if (strlen(this->errorMessage) > 0)
	{
		delete[] filteredData;
		return false;
	}

	Image* image = new Image(headerChunk->width, headerChunk->height);
	assetList.AddTail(image);

	unsigned char* filterTypeArray = new unsigned char[headerChunk->height];
	unsigned char* filterDataPtr = filteredData;
	for (unsigned int row = 0; row < image->GetHeight(); row++)
	{
		filterTypeArray[row] = *filterDataPtr++;

		for (unsigned int col = 0; col < image->GetWidth(); col++)
		{
			Image::Pixel* pixel = image->GetPixel(Image::Location{ row, col });
			pixel->color = image->MakeColor(filterDataPtr[0], filterDataPtr[1], filterDataPtr[2], 0);
			filterDataPtr += 3;
		}
	}

	// TODO: Unfilter the image here...

	delete[] filteredData;
	delete[] filterTypeArray;

	return true;
}

/*virtual*/ bool PNGFormat::SaveAssets(const char* filePath, const List<Asset*>& assetList)
{
	// TODO: Write this.
	return true;
}

PNGFormat::Chunk* PNGFormat::MakeChunkForChunkType(Stream& stream, bool& isCritical)
{
	Chunk* chunk = nullptr;

	unsigned int size = 0;
	if (stream.Read((unsigned char*)&size, 4))
	{
		size = FRUMPY_ENDIAN_SWAP4(size);

		char chunkType[5];
		stream.Read((unsigned char*)chunkType, 4);
		chunkType[4] = '\0';

		isCritical = isupper(chunkType[0]) ? true : false;

		if (0 == strcmp(chunkType, "IHDR"))
			chunk = new HeaderChunk();
		else if (0 == strcmp(chunkType, "IDAT"))
			chunk = new ImageDataChunk();
		else
			chunk = new OpaqueChunk();

		chunk->size = size;
	}

	return chunk;
}

//---------------------------- PNGFormat::Chunk ----------------------------

PNGFormat::Chunk::Chunk()
{
}

/*virtual*/ PNGFormat::Chunk::~Chunk()
{
}

//---------------------------- PNGFormat::HeaderChunk ----------------------------

PNGFormat::OpaqueChunk::OpaqueChunk()
{
	this->buffer = nullptr;
}

/*virtual*/ PNGFormat::OpaqueChunk::~OpaqueChunk()
{
	delete[] this->buffer;
}

/*virtual*/ bool PNGFormat::OpaqueChunk::ReadData(Stream& stream)
{
	if (this->size > 0)
	{
		this->buffer = new unsigned char[this->size];
		stream.Read(this->buffer, this->size);
	}

	int skip = 0;
	stream.Read((unsigned char*)&skip, 4);	// Skip CRC.

	return true;
}

/*virtual*/ bool PNGFormat::OpaqueChunk::WriteData(Stream& stream) const
{
	return false;
}

//---------------------------- PNGFormat::HeaderChunk ----------------------------
		
PNGFormat::HeaderChunk::HeaderChunk()
{
	this->width = 0;
	this->height = 0;
	this->bitDepth = 0;
	this->colorType = 0;
	this->compressionMethod = 0;
	this->filterMethod = 0;
	this->interlaceMethod = 0;
}

/*virtual*/ PNGFormat::HeaderChunk::~HeaderChunk()
{
}

/*virtual*/ bool PNGFormat::HeaderChunk::ReadData(Stream& stream)
{
	stream.Read((unsigned char*)&this->width, 4);
	stream.Read((unsigned char*)&this->height, 4);
	this->width = FRUMPY_ENDIAN_SWAP4(this->width);
	this->height = FRUMPY_ENDIAN_SWAP4(this->height);

	stream.Read(&this->bitDepth, 1);
	stream.Read(&this->colorType, 1);
	stream.Read(&this->compressionMethod, 1);
	stream.Read(&this->filterMethod, 1);
	stream.Read(&this->interlaceMethod, 1);

	int skip = 0;
	stream.Read((unsigned char*)&skip, 4);	// Skip CRC.

	return true;
}

/*virtual*/ bool PNGFormat::HeaderChunk::WriteData(Stream& stream) const
{
	return false;
}

//---------------------------- PNGFormat::HeaderChunk ----------------------------

PNGFormat::ImageDataChunk::ImageDataChunk()
{
}

/*virtual*/ PNGFormat::ImageDataChunk::~ImageDataChunk()
{
}