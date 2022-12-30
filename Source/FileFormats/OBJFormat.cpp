#include "OBJFormat.h"
#include "../FileAssets/Mesh.h"
#include "../Triangle.h"
#include <map>

using namespace Frumpy;

OBJFormat::OBJFormat()
{
	this->data = new Data();
	this->totalVertices = 0;
	this->totalFaces = 0;
}

/*virtual*/ OBJFormat::~OBJFormat()
{
	delete this->data;
}

/*virtual*/ const char* OBJFormat::SupportedExtension()
{
	return "OBJ";
}

/*virtual*/ bool OBJFormat::LoadAssets(const char* filePath, List<AssetManager::Asset*>& assetList)
{
	std::ifstream fileStream(filePath, std::ios::in);
	if (fileStream.is_open())
	{
		this->data->pointArray.clear();
		this->data->normalArray.clear();
		this->data->colorArray.clear();
		this->data->texCoordsArray.clear();
		this->data->polygonArray.clear();
		this->data->name = "?";

		std::string line;
		while (std::getline(fileStream, line))
		{
			std::vector<std::string> tokenArray;
			this->TokenizeLine(line, ' ', tokenArray, true);
			this->ProcessTokenizedLine(tokenArray, assetList);
		}

		this->FlushMesh(assetList);

		fileStream.close();
	}

	return true;
}

void OBJFormat::TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens)
{
	std::stringstream stringStream(line);
	std::string token;
	while (std::getline(stringStream, token, delimeter))
		if (!stripEmptyTokens || token.size() > 0)
			tokenArray.push_back(token);
}

void OBJFormat::ProcessTokenizedLine(const std::vector<std::string>& tokenArray, List<AssetManager::Asset*>& assetList)
{
	if (tokenArray.size() == 0 || tokenArray[0] == "#")
		return;

	if (tokenArray[0] == "v")
	{
		Vector point;
		point.x = (tokenArray.size() > 1) ? ::atof(tokenArray[1].c_str()) : 0.0;
		point.y = (tokenArray.size() > 2) ? ::atof(tokenArray[2].c_str()) : 0.0;
		point.z = (tokenArray.size() > 3) ? ::atof(tokenArray[3].c_str()) : 0.0;
		this->data->pointArray.push_back(point);
	}
	else if (tokenArray[0] == "vt")
	{
		Vector texCoords;
		texCoords.x = (tokenArray.size() > 1) ? ::atof(tokenArray[1].c_str()) : 0.0;
		texCoords.y = (tokenArray.size() > 2) ? ::atof(tokenArray[2].c_str()) : 0.0;
		texCoords.z = (tokenArray.size() > 3) ? ::atof(tokenArray[3].c_str()) : 0.0;
		this->data->texCoordsArray.push_back(texCoords);
	}
	else if (tokenArray[0] == "vn")
	{
		Vector normal;
		normal.x = (tokenArray.size() > 1) ? ::atof(tokenArray[1].c_str()) : 0.0;
		normal.y = (tokenArray.size() > 2) ? ::atof(tokenArray[2].c_str()) : 0.0;
		normal.z = (tokenArray.size() > 3) ? ::atof(tokenArray[3].c_str()) : 0.0;
		this->data->normalArray.push_back(normal);
	}
	else if (tokenArray[0] == "f" && tokenArray.size() > 1)
	{
		ConvexPolygon polygon;

		for (const std::string& token : tokenArray)
		{
			if (token == "f")
				continue;

			std::vector<std::string> vertexTokenArray;
			this->TokenizeLine(token, '/', vertexTokenArray, false);

			Vertex vertex;

			int point_i = (vertexTokenArray.size() > 0 && vertexTokenArray[0].size() > 0) ? ::atoi(vertexTokenArray[0].c_str()) : INT_MAX;
			int texCoords_i = (vertexTokenArray.size() > 1 && vertexTokenArray[1].size() > 0) ? ::atoi(vertexTokenArray[1].c_str()) : INT_MAX;
			int normal_i = (vertexTokenArray.size() > 2 && vertexTokenArray[2].size() > 0) ? ::atoi(vertexTokenArray[2].c_str()) : INT_MAX;

			this->LookupAndAssign(this->data->pointArray, point_i, vertex.objectSpacePoint);
			this->LookupAndAssign(this->data->texCoordsArray, texCoords_i, vertex.texCoords);
			this->LookupAndAssign(this->data->normalArray, normal_i, vertex.objectSpaceNormal);

			polygon.vertexArray.push_back(vertex);
		}

		this->data->polygonArray.push_back(polygon);
	}
	else if (tokenArray[0] == "g" && tokenArray.size() > 1)
	{
		this->FlushMesh(assetList);
		this->data->name = tokenArray[1];
	}
}

void OBJFormat::FlushMesh(List<AssetManager::Asset*>& assetList)
{
	if (this->data->polygonArray.size() > 0)
	{
		// Create a named mesh asset.
		Mesh* mesh = new Mesh();
		strcpy_s(mesh->name, sizeof(mesh->name), this->data->name.c_str());
		
		// Convert the list of polygons to a list of triangles.
		std::vector<ConvexPolygon> triangleArray;
		for (ConvexPolygon& polygon : this->data->polygonArray)
			polygon.Tesselate(triangleArray);

		// Build the vertex buffer.
		unsigned int i = 0;
		std::vector<Vertex> vertexBuffer;
		std::map<std::string, unsigned int> vertexMap;
		for (ConvexPolygon& polygon : triangleArray)
		{
			for (Vertex& vertex : polygon.vertexArray)
			{
				std::string key = vertex.MakeKey();
				std::map<std::string, unsigned int>::iterator iter = vertexMap.find(key);
				if (iter == vertexMap.end())
				{
					vertexBuffer.push_back(vertex);
					vertexMap.insert(std::pair<std::string, unsigned int>(key, i++));
				}
			}
		}

		// Build the index buffer.
		std::vector<unsigned int> indexBuffer;
		for (ConvexPolygon& polygon : triangleArray)
		{
			for (Vertex& vertex : polygon.vertexArray)
			{
				std::string key = vertex.MakeKey();
				std::map<std::string, unsigned int>::iterator iter = vertexMap.find(key);
				if (iter != vertexMap.end())
					indexBuffer.push_back(iter->second);
				else
					indexBuffer.push_back(0);
			}
		}

		// Populate the vertex buffer on the mesh.
		mesh->SetVertexBufferSize(vertexBuffer.size());
		for (i = 0; i < vertexBuffer.size(); i++)
			*mesh->GetVertex(i) = vertexBuffer[i];
		
		// Populate the index buffer on the mesh.
		mesh->SetIndexBufferSize(indexBuffer.size());
		for (i = 0; i < indexBuffer.size(); i++)
			mesh->SetIndex(i, indexBuffer[i]);

		// Finally, add the mesh to the list of returned assets.
		assetList.AddTail(mesh);

		// Start over with a new empty list of polygons.
		this->data->polygonArray.clear();
	}
}

void OBJFormat::LookupAndAssign(const std::vector<Vector>& vectorArray, int i, Vector& result)
{
	if (vectorArray.size() > 0 && i != INT_MAX)
	{
		// 1, 2, 3, ... becomes 0, 1, 2, ...
		// -1, -2, -3, ... becomes N-1, N-2, N-3, ...
		if (i > 0)
			i--;
		else if (i < 0)
			i = (int)vectorArray.size() + i;

		// Clamp the offset to be in range.
		if (i < 0)
			i = 0;
		else if (i >= (signed)vectorArray.size())
			i = (int)vectorArray.size() - 1;

		result = vectorArray[i];
	}
}

void OBJFormat::ConvexPolygon::Tesselate(std::vector<ConvexPolygon>& triangleArray) const
{
	std::vector<Vertex> vertexArrayCopy = this->vertexArray;

	while (vertexArrayCopy.size() > 2)
	{
		double smallestDifferenceFromIdeal = FLT_MAX;

		// This doesn't necessarily produce the best tesselation, but it may be good enough for now.
		unsigned int j = 0;
		if (vertexArrayCopy.size() > 3)
		{
			for (unsigned int i = 0; i < vertexArrayCopy.size(); i++)
			{
				Triangle triangle;
				triangle.vertex[0] = vertexArrayCopy[i].objectSpacePoint;
				triangle.vertex[1] = vertexArrayCopy[(i + 1) % vertexArrayCopy.size()].objectSpacePoint;
				triangle.vertex[2] = vertexArrayCopy[(i + 2) % vertexArrayCopy.size()].objectSpacePoint;

				double smallestAngle = FRUMPY_RADS_TO_DEGS(triangle.SmallestInteriorAngle());
				double differenceFromIdeal = fabs(60.0 - smallestAngle);
				if (differenceFromIdeal < smallestDifferenceFromIdeal)
				{
					smallestDifferenceFromIdeal = differenceFromIdeal;
					j = i;
				}
			}
		}

		ConvexPolygon triangle;
		triangle.vertexArray.push_back(vertexArrayCopy[j]);
		triangle.vertexArray.push_back(vertexArrayCopy[(j + 1) % vertexArrayCopy.size()]);
		triangle.vertexArray.push_back(vertexArrayCopy[(j + 2) % vertexArrayCopy.size()]);
		triangleArray.push_back(triangle);

		std::vector<Vertex>::iterator iter = vertexArrayCopy.begin() + ((j + 1) % vertexArrayCopy.size());
		vertexArrayCopy.erase(iter);
	}
}

/*virtual*/ bool OBJFormat::SaveAssets(const char* filePath, const List<AssetManager::Asset*>& assetList)
{
	std::ofstream fileStream(filePath);
	if (!fileStream.is_open())
		return false;

	fileStream << "#\n";
	fileStream << "# Generated by Frumpy!\n";
	fileStream << "#\n\n";

	this->totalVertices = 0;
	this->totalFaces = 0;

	for (const List<AssetManager::Asset*>::Node* node = assetList.GetHead(); node; node = node->GetNext())
	{
		const AssetManager::Asset* asset = node->value;

		const Mesh* mesh = dynamic_cast<const Mesh*>(asset);
		if (mesh)
			this->DumpMesh(fileStream, mesh);

		/*
		const Polyline* polyline = dynamic_cast<const Polyline*>(asset);
		if (polyline)
			this->DumpPolyline(fileStream, polyline);
		*/
	}

	fileStream << "# Total vertices: " << this->totalVertices << "\n";
	fileStream << "# Total faces:    " << this->totalFaces << "\n";

	fileStream.close();
	return true;
}

void OBJFormat::DumpMesh(std::ofstream& fileStream, const Mesh* mesh)
{
	for (unsigned int i = 0; i < mesh->GetVertexBufferSize(); i++)
	{
		// TODO: Output vertex colors too?
		const Vertex* vertex = mesh->GetVertex(i);
		fileStream << "v " << vertex->objectSpacePoint.x << " " << vertex->objectSpacePoint.y << " " << vertex->objectSpacePoint.z << "\n";
	}

	fileStream << "\n";

	for (unsigned int i = 0; i < mesh->GetVertexBufferSize(); i++)
	{
		const Vertex* vertex = mesh->GetVertex(i);
		fileStream << "vt " << vertex->texCoords.x << " " << vertex->texCoords.y << " " << vertex->texCoords.z << "\n";
	}

	fileStream << "\n";

	for (unsigned int i = 0; i < mesh->GetVertexBufferSize(); i++)
	{
		const Vertex* vertex = mesh->GetVertex(i);
		fileStream << "vn " << vertex->objectSpaceNormal.x << " " << vertex->objectSpaceNormal.y << " " << vertex->objectSpaceNormal.z << "\n";
	}

	fileStream << "\n";
	fileStream << "g " << mesh->name << "\n\n";

	for (unsigned int i = 0; i < mesh->GetIndexBufferSize(); i += 3)
	{
		fileStream << "f ";

		for (unsigned int j = 0; j < 3; j++)
		{
			unsigned int vertex_i = this->totalVertices + mesh->GetIndex(i + j) + 1;	// Needs to be 1-based, not 0-based.
			fileStream << vertex_i << "/" << vertex_i << "/" << vertex_i << " ";
		}

		fileStream << "\n";
	}

	fileStream << "\n";

	this->totalVertices += mesh->GetVertexBufferSize();
	this->totalFaces += mesh->GetIndexBufferSize() / 3;
}

/*
void OBJFormat::DumpPolyline(std::ofstream& fileStream, const Polyline* polyline)
{
	for (int i = 0; i < (int)polyline->vertexArray->size(); i++)
	{
		const Vector& vertex = (*polyline->vertexArray)[i];
		fileStream << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
	}

	fileStream << "\nl";

	for (int i = 0; i < (int)polyline->vertexArray->size(); i++)
		fileStream << " " << i + this->totalVertices + 1;

	fileStream << "\n\n";

	this->totalVertices += (int)polyline->vertexArray->size();
}
*/