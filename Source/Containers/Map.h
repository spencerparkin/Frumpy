#pragma once

#include "Defines.h"
#include "Containers/List.h"

namespace Frumpy
{
	template<typename Type>
	class FRUMPY_API Map
	{
	private:

		struct Entry
		{
			Type type;
			unsigned int hash;
		};

		typedef List<Entry> EntryList;

	public:
		Map()
		{
			this->entryListArraySize = 512;
			this->entryListArray = new EntryList[this->entryListArraySize];
		}

		virtual ~Map()
		{
			delete[] this->entryListArray;
		}

		bool Find(const char* key, Type& foundType)
		{
			typename EntryList::Node* node = this->Find(key, false, false);
			if (!node)
				return false;
			foundType = node->value.type;
			return true;
		}

		bool Insert(const char* key, Type type)
		{
			typename EntryList::Node* node = this->Find(key, true, false);
			node->value.type = type;
			return true;
		}

		bool Remove(const char* key)
		{
			typename EntryList::Node* node = this->Find(key, false, true);
			return node ? true : false;
		}

		void Clear()
		{
			for (unsigned int i = 0; i < this->entryListArraySize; i++)
				this->entryListArray[i].Clear();
		}

		void Delete()
		{
			for (unsigned int i = 0; i < this->entryListArraySize; i++)
			{
				EntryList& entryList = this->entryListArray[i];
				for (typename EntryList::Node* node = entryList.GetHead(); node; node = node->GetNext())
					delete node->value.type;
				entryList.Clear();
			}
		}

	private:

		unsigned long CalcHash(const char* key)
		{
			// A hash by Bernstein...
			unsigned long hash = 5381;
			while (true)
			{
				unsigned long c = *key++;
				if (c == '\0')
					break;

				hash = ((hash << 5) + hash) + c;
			}

			return hash;
		}

		EntryList::Node* Find(const char* key, bool insertIfNotFound, bool removeIfFound)
		{
			unsigned long hash = this->CalcHash(key);
			unsigned long i = hash % (unsigned long)(this->entryListArraySize);
			EntryList& entryList = this->entryListArray[i];
			for (typename EntryList::Node* node = entryList.GetHead(); node; node = node->GetNext())
			{
				if (node->value.hash == hash)
				{
					if (removeIfFound)
					{
						entryList.Remove(node);
						return nullptr;
					}

					return node;
				}
			}
			
			if (insertIfNotFound)
			{
				Entry entry;
				entry.hash = hash;
				entryList.AddTail(entry);
				return entryList.GetTail();
			}

			return nullptr;
		}

		EntryList* entryListArray;
		unsigned int entryListArraySize;
	};
}