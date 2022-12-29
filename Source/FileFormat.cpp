#include "FileFormat.h"

using namespace Frumpy;

FileFormat::FileFormat()
{
	this->errorMessage[0] = '\0';
}

/*virtual*/ FileFormat::~FileFormat()
{
}

FileFormat::Asset::Asset()
{
	this->name = new std::string();
}

/*virtual*/ FileFormat::Asset::~Asset()
{
	delete this->name;
}

/*virtual*/ FileFormat::Asset* FileFormat::Asset::Clone() const
{
	return nullptr;
}