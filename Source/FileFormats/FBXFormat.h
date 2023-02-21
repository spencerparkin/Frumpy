#pragma once

#include "AssetManager.h"
#include <string>
#include <vector>
#include <list>

namespace Frumpy
{
	class FRUMPY_API FBXFormat : public AssetManager::FileFormat
	{
	public:
		FBXFormat();
		virtual ~FBXFormat();

		virtual const char* SupportedExtension() override;

		virtual bool LoadAssets(const char* filePath, List<AssetManager::Asset*>& assetList) override;
		virtual bool SaveAssets(const char* filePath, const List<const AssetManager::Asset*>& assetList) override;

	protected:

		class PropertyBase
		{
		public:
			enum class Type
			{
				INT,
				FLOAT,
				STRING
			};
			Type type;
		};

		template<typename T>
		class Property : public PropertyBase
		{
		public:
			T value;
		};

		class PropertyInt : public Property<int>
		{
		public:
			PropertyInt(int value)
			{
				this->type = Type::INT;
				this->value = value;
			}
		};

		class PropertyFloat : public Property<double>
		{
		public:
			PropertyFloat(double value)
			{
				this->type = Type::FLOAT;
				this->value = value;
			}
		};

		class PropertyString : public Property<std::string>
		{
		public:
			PropertyString(const std::string& value)
			{
				this->type = Type::STRING;
				this->value = value;
			}
		};

		class Node
		{
		public:
			Node();
			virtual ~Node();

			std::string name;
			std::vector<PropertyBase*> propertyArray;
			std::vector<Node*> childrenArray;
		};

		struct Token
		{
			enum Type
			{
				IDENTIFIER,
				COLON,
				DELIMETER,
				OPEN_CURLY,
				CLOSE_CURLY,
				STRING,
				NUMBER
			};
			Type type;
			std::string data;
		};

		Node* ParseAsciiFbxFile(const char* filePath);
		bool TokenizeAsciiFbxFile(const char* filePath, std::list<Token>& tokenList);
		bool EatTokensFromLine(const char* buffer, int bufferSize, std::list<Token>& tokenList);
		bool ParseNodeArray(std::list<Token>& tokenList, std::vector<Node*>& nodeArray);
		bool ParseNode(std::list<Token>& tokenList, Node* node);
		bool LoadAssetsFromTree(const Node* rootNode, List<AssetManager::Asset*>& assetList);
	};
}