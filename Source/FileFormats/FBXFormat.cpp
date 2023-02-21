#include "FBXFormat.h"
#include <fstream>

using namespace Frumpy;

FBXFormat::FBXFormat()
{
}

/*virtual*/ FBXFormat::~FBXFormat()
{
}

/*virtual*/ const char* FBXFormat::SupportedExtension()
{
	return "FBX";
}

/*virtual*/ bool FBXFormat::LoadAssets(const char* filePath, List<AssetManager::Asset*>& assetList)
{
	Node* rootNode = ParseAsciiFbxFile(filePath);
	if (!rootNode)
		return false;

	bool success = this->LoadAssetsFromTree(rootNode, assetList);
	delete rootNode;
	return success;
}

/*virtual*/ bool FBXFormat::SaveAssets(const char* filePath, const List<const AssetManager::Asset*>& assetList)
{
	// I'm not likely to ever get around to supporting this.
	return false;
}

bool FBXFormat::LoadAssetsFromTree(const Node* rootNode, List<AssetManager::Asset*>& assetList)
{
	// TODO: Decipher the FBX data here.

	return true;
}

FBXFormat::Node* FBXFormat::ParseAsciiFbxFile(const char* filePath)
{
	std::list<Token> tokenList;
	if (!this->TokenizeAsciiFbxFile(filePath, tokenList))
		return nullptr;

	Node* rootNode = new Node();
	rootNode->name = "root";
	if (!this->ParseNodeArray(tokenList, rootNode->childrenArray))
	{
		delete rootNode;
		rootNode = nullptr;
	}

	return rootNode;
}

bool FBXFormat::ParseNodeArray(std::list<Token>& tokenList, std::vector<Node*>& nodeArray)
{
	nodeArray.clear();
	while (tokenList.size() > 0)
	{
		Token token = *tokenList.begin();
		if (token.type == Token::CLOSE_CURLY)
			break;

		Node* node = new Node();
		if (this->ParseNode(tokenList, node))
			nodeArray.push_back(node);
		else
		{
			delete node;
			return false;
		}
	}

	return true;
}

bool FBXFormat::ParseNode(std::list<Token>& tokenList, Node* node)
{
	if (tokenList.size() < 4)
		return false;

	Token token = *tokenList.begin();
	if (token.type != Token::IDENTIFIER)
		return false;

	node->name = token.data;

	tokenList.erase(tokenList.begin());
	token = *tokenList.begin();
	if (token.type != Token::COLON)
		return false;

	// Read off all the properties.
	bool expectingComma = false;
	tokenList.erase(tokenList.begin());
	while (tokenList.size() > 0)
	{
		// Are we at the end of the property list?
		token = *tokenList.begin();
		if (token.type == Token::OPEN_CURLY || token.type == Token::IDENTIFIER)
		{
			// Special case: Some property lists consist of a single identifier.
			if (node->propertyArray.size() == 0 && token.type == Token::IDENTIFIER)
			{
				tokenList.erase(tokenList.begin());
				node->propertyArray.push_back(new PropertyString(token.data));
				continue;
			}

			break;
		}

		if (expectingComma)
		{
			if (token.type != Token::DELIMETER)
				return false;	// Expected comma, but didn't see one.

			tokenList.erase(tokenList.begin());
			expectingComma = false;
			continue;
		}

		PropertyBase* newProperty = nullptr;
		switch (token.type)
		{
			case Token::NUMBER:
			{
				double floatValue = atof(token.data.c_str());
				if (floatValue == ::round(floatValue))
					newProperty = new PropertyInt(int(floatValue));
				else
					newProperty = new PropertyFloat(floatValue);

				break;
			}
			case Token::STRING:
			{
				newProperty = new PropertyString(token.data);
				break;
			}
			default:
			{
				// Unexpected token encountered.
				return false;
			}
		}

		if (!newProperty)
			return false;

		node->propertyArray.push_back(newProperty);
		expectingComma = true;
		tokenList.erase(tokenList.begin());
	}

	// Are there sub-nodes?
	if (token.type == Token::OPEN_CURLY)
	{
		tokenList.erase(tokenList.begin());

		this->ParseNodeArray(tokenList, node->childrenArray);

		if (tokenList.size() == 0)
			return false;

		token = *tokenList.begin();
		if (token.type != Token::CLOSE_CURLY)
			return false;	// Expected close curly, but didn't see one.

		tokenList.erase(tokenList.begin());
	}

	return true;
}

bool FBXFormat::TokenizeAsciiFbxFile(const char* filePath, std::list<Token>& tokenList)
{
	std::ifstream fileStream(filePath, std::ios::in);
	if (!fileStream.is_open())
		return false;
	
	tokenList.clear();

	std::string line;
	while (std::getline(fileStream, line))
		if (!this->EatTokensFromLine(line.c_str(), line.size(), tokenList))
			return false;
	
	return tokenList.size() > 0;
}

bool FBXFormat::EatTokensFromLine(const char* buffer, int bufferSize, std::list<Token>& tokenList)
{
	int i = 0;
	while (i < bufferSize)
	{
		// Skip any leading white space.
		while (i < bufferSize && ::isspace(buffer[i]))
			i++;

		// Reached end of line?
		if (i >= bufferSize)
			return true;

		// We're done in this case, because the remainder of the line is a comment.
		if (buffer[i] == ';')
			return true;

		// Make a new token.
		Token token;
		token.data = "";

		if (::isalpha(buffer[i]) || buffer[i] == '_')	// Bite off an identifier.
		{
			token.type = Token::IDENTIFIER;
			while (i < bufferSize && (::isalnum(buffer[i]) || buffer[i] == '_'))
				token.data += buffer[i++];
		}
		else if (::isdigit(buffer[i]) || buffer[i] == '-' || buffer[i] == '*' || buffer[i] == 'e')	// Bite off a number.
		{
			token.type = Token::NUMBER;
			while (i < bufferSize && (::isdigit(buffer[i]) || buffer[i] == '-' || buffer[i] == '*' || buffer[i] == '.' || buffer[i] == 'e'))
			{
				if (buffer[i] != '*')	// TODO: What does the star mean?
					token.data += buffer[i];
				i++;
			}
		}
		else if (buffer[i] == '"')	// Bite off a string.
		{
			token.type = Token::STRING;
			i++;
			while (i < bufferSize && buffer[i] != '"')
				token.data += buffer[i++];
			if (i < bufferSize)
				i++;
		}
		else if (buffer[i] == ':')
		{
			token.type = Token::COLON;
			token.data = ":";
			i++;
		}
		else if (buffer[i] == ',')
		{
			token.type = Token::DELIMETER;
			token.data = ",";
			i++;
		}
		else if (buffer[i] == '{')
		{
			token.type = Token::OPEN_CURLY;
			token.data = "{";
			i++;
		}
		else if (buffer[i] == '}')
		{
			token.type = Token::CLOSE_CURLY;
			token.data = "}";
			i++;
		}
		else	// Unknown token encountered!
		{
			return false;
		}

		tokenList.push_back(token);
	}

	return true;
}

FBXFormat::Node::Node()
{
}

/*virtual*/ FBXFormat::Node::~Node()
{
	for (int i = 0; i < (signed)this->propertyArray.size(); i++)
		delete this->propertyArray[i];

	for (Node* childNode : this->childrenArray)
		delete childNode;
}