#pragma once

#include "../FileFormat.h"
#include <fstream>

namespace Frumpy
{
	class Mesh;
	class Stream;

	class FRUMPY_API PNGFormat : public FileFormat
	{
	public:
		PNGFormat();
		virtual ~PNGFormat();

		virtual const char* SupportedExtension() override;

		virtual bool LoadAssets(const char* filePath, List<Asset*>& assetList) override;
		virtual bool SaveAssets(const char* filePath, const List<Asset*>& assetList) override;

		class Chunk
		{
		public:
			Chunk();
			virtual ~Chunk();

			virtual bool ReadData(Stream& stream) = 0;
			virtual bool WriteData(Stream& stream) const = 0;

			unsigned int size;
		};

		Chunk* MakeChunkForChunkType(Stream& stream, bool& isCritical);

		class OpaqueChunk : public Chunk
		{
		public:
			OpaqueChunk();
			virtual ~OpaqueChunk();

			virtual bool ReadData(Stream& stream) override;
			virtual bool WriteData(Stream& stream) const override;

			unsigned char* buffer;
		};

		class HeaderChunk : public Chunk
		{
		public:
			HeaderChunk();
			virtual ~HeaderChunk();

			virtual bool ReadData(Stream& stream) override;
			virtual bool WriteData(Stream& stream) const override;

			unsigned int width;
			unsigned int height;
			unsigned char bitDepth;
			unsigned char colorType;
			unsigned char compressionMethod;
			unsigned char filterMethod;
			unsigned char interlaceMethod;
		};

		class ImageDataChunk : public OpaqueChunk
		{
		public:
			ImageDataChunk();
			virtual ~ImageDataChunk();
		};

		typedef List<Chunk*> ChunkList;

		template<typename ChunkType>
		ChunkType* FindChunkByType()
		{
			for (ChunkList::Node* node = this->chunkList.GetHead(); node; node = node->GetNext())
			{
				ChunkType* chunk = dynamic_cast<ChunkType*>(node->value);
				if (chunk)
					return chunk;
			}

			return nullptr;
		}

		template<typename ChunkType>
		void GatherAllChunksOfType(List<ChunkType*>& givenChunkList)
		{
			for (ChunkList::Node* node = this->chunkList.GetHead(); node; node = node->GetNext())
			{
				ChunkType* chunk = dynamic_cast<ChunkType*>(node->value);
				if (chunk)
					givenChunkList.AddTail(chunk);
			}
		}

		unsigned int ReadUInt();

		ChunkList chunkList;
	};
}