#pragma once

#include "../AssetManager.h"
#include "../Vector.h"
#include "../Vertex.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

namespace Frumpy
{
	class Mesh;

	class FRUMPY_API OBJFormat : public AssetManager::FileFormat
	{
	public:
		OBJFormat();
		virtual ~OBJFormat();

		virtual const char* SupportedExtension() override;

		virtual bool LoadAssets(const char* filePath, List<AssetManager::Asset*>& assetList) override;
		virtual bool SaveAssets(const char* filePath, const List<const AssetManager::Asset*>& assetList) override;

	private:

		struct ConvexPolygon
		{
			std::vector<Vertex> vertexArray;

			void Tesselate(std::vector<ConvexPolygon>& triangleArray) const;
		};

		struct Data
		{
			std::vector<Vector> pointArray;
			std::vector<Vector> normalArray;
			std::vector<Vector> colorArray;
			std::vector<Vector> texCoordsArray;
			std::vector<ConvexPolygon> polygonArray;
			std::string name;
		};

		Data* data;
		int totalVertices;
		int totalFaces;

		void TokenizeLine(const std::string& line, char delimeter, std::vector<std::string>& tokenArray, bool stripEmptyTokens);
		void ProcessTokenizedLine(const std::vector<std::string>& tokenArray, List<AssetManager::Asset*>& assetList);
		void LookupAndAssign(const std::vector<Vector>& vectorArray, int i, Vector& result);
		void FlushMesh(List<AssetManager::Asset*>& assetList);
		void DumpMesh(std::ofstream& fileStream, const Mesh* mesh);
		//void DumpPolyline(std::ofstream& fileStream, const Polyline* polyline);
	};
}