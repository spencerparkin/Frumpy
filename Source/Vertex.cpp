#include "Vertex.h"
#include <sstream>

using namespace Frumpy;

Vertex::Vertex()
{
}

/*virtual*/ Vertex::~Vertex()
{
}

std::string Vertex::MakeKey() const
{
	std::stringstream stringStream;
	stringStream <<
		this->objectSpacePoint.x << "|" <<
		this->objectSpacePoint.y << "|" <<
		this->objectSpacePoint.z << "|" <<
		this->objectSpaceNormal.x << "|" <<
		this->objectSpaceNormal.y << "|" <<
		this->objectSpaceNormal.z << "|" <<
		this->texCoords.x << "|" <<
		this->texCoords.y << "|" <<
		this->texCoords.z << "|" <<
		this->color.r << "|" <<
		this->color.g << "|" <<
		this->color.b << "|" <<
		this->color.a;
	std::string key = stringStream.str();
	return key;
}