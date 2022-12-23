#include "FileFormat.h"

using namespace Frumpy;

FileFormat::FileFormat()
{
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