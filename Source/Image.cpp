#include "Image.h"
#include "Vertex.h"
#include "Plane.h"
#include "Triangle.h"
#include <math.h>

using namespace Frumpy;

Image::Image()
{
	this->pixelData = nullptr;
	this->width = 0;
	this->height = 0;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->pixelData = nullptr;
	this->SetWidthAndHeight(width, height);
}

/*virtual*/ Image::~Image()
{
	delete[] this->pixelData;
}

void Image::SetWidthAndHeight(unsigned int width, unsigned int height)
{
	if (width != this->width || height != this->height)
	{
		delete[] this->pixelData;
		this->pixelData = nullptr;
		this->width = width;
		this->height = height;

		if (this->width > 0 && this->height > 0)
		{
			unsigned int pixelDataSize = this->GetRawPixelBufferSize();
			this->pixelData = new Pixel[pixelDataSize];
		}
	}
}

void Image::Clear(const Pixel& pixel)
{
	for (unsigned int i = 0; i < this->GetRawPixelBufferSize(); i++)
		this->pixelData[i] = pixel;
}

bool Image::ValidLocation(const Location& location) const
{
	if (location.row >= this->height)
		return false;

	if (location.col >= this->width)
		return false;

	return true;
}

Image::Pixel* Image::GetPixel(const Location& location)
{
	if (!this->ValidLocation(location))
		return nullptr;

	return &this->pixelData[location.row * this->width + location.col];
}

const Image::Pixel* Image::GetPixel(const Location& location) const
{
	return const_cast<Image*>(this)->GetPixel(location);
}

void Image::CalcImageMatrix(Matrix& imageMatrix) const
{
	imageMatrix.Identity();
	imageMatrix.ele[0][0] = double(this->width) / 2.0;
	imageMatrix.ele[0][3] = double(this->width) / 2.0;
	imageMatrix.ele[1][1] = double(this->height) / 2.0;
	imageMatrix.ele[1][3] = double(this->height) / 2.0;
}

void Image::RenderTriangle(const Vertex& vertexA, const Vertex& vertexB, const Vertex& vertexC, const PipelineMatrices& pipelineMatrices, Image& depthBuffer)
{
	struct Edge
	{
		const Vector* vertexA;
		const Vector* vertexB;
	};

	Edge minEdges[2];
	Edge maxEdges[2];

	double minY = FRUMPY_MIN(FRUMPY_MIN(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y), vertexC.imageSpacePoint.y);
	double maxY = FRUMPY_MAX(FRUMPY_MAX(vertexA.imageSpacePoint.y, vertexB.imageSpacePoint.y), vertexC.imageSpacePoint.y);

	double midY = 0.0;
	
	if (minY == vertexA.imageSpacePoint.y)
	{
		if (maxY == vertexB.imageSpacePoint.y)
			midY = vertexC.imageSpacePoint.y;
		else
			midY = vertexB.imageSpacePoint.y;

		minEdges[0].vertexA = &vertexA.imageSpacePoint;
		minEdges[0].vertexB = &vertexB.imageSpacePoint;
		minEdges[1].vertexA = &vertexA.imageSpacePoint;
		minEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else if(minY == vertexB.imageSpacePoint.y)
	{
		if (maxY == vertexA.imageSpacePoint.y)
			midY = vertexC.imageSpacePoint.y;
		else
			midY = vertexA.imageSpacePoint.y;

		minEdges[0].vertexA = &vertexB.imageSpacePoint;
		minEdges[0].vertexB = &vertexA.imageSpacePoint;
		minEdges[1].vertexA = &vertexB.imageSpacePoint;
		minEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else
	{
		if (maxY == vertexA.imageSpacePoint.y)
			midY = vertexB.imageSpacePoint.y;
		else
			midY = vertexA.imageSpacePoint.y;

		minEdges[0].vertexA = &vertexC.imageSpacePoint;
		minEdges[0].vertexB = &vertexA.imageSpacePoint;
		minEdges[1].vertexA = &vertexC.imageSpacePoint;
		minEdges[1].vertexB = &vertexB.imageSpacePoint;
	}

	if (maxY == vertexA.imageSpacePoint.y)
	{
		maxEdges[0].vertexA = &vertexA.imageSpacePoint;
		maxEdges[0].vertexB = &vertexB.imageSpacePoint;
		maxEdges[1].vertexA = &vertexA.imageSpacePoint;
		maxEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else if (maxY == vertexB.imageSpacePoint.y)
	{
		maxEdges[0].vertexA = &vertexB.imageSpacePoint;
		maxEdges[0].vertexB = &vertexA.imageSpacePoint;
		maxEdges[1].vertexA = &vertexB.imageSpacePoint;
		maxEdges[1].vertexB = &vertexC.imageSpacePoint;
	}
	else
	{
		maxEdges[0].vertexA = &vertexC.imageSpacePoint;
		maxEdges[0].vertexB = &vertexA.imageSpacePoint;
		maxEdges[1].vertexA = &vertexC.imageSpacePoint;
		maxEdges[1].vertexB = &vertexB.imageSpacePoint;
	}

	int minRow = int(floor(minY));
	int maxRow = int(ceil(maxY));
	int midRow = int(round(midY));

	minRow = FRUMPY_MAX(minRow, 0);
	maxRow = FRUMPY_MIN(maxRow, signed(this->height));

	if (minRow >= maxRow)
		return;

	Triangle triangle;
	triangle.vertex[0] = vertexA.cameraSpacePoint;
	triangle.vertex[1] = vertexB.cameraSpacePoint;
	triangle.vertex[2] = vertexC.cameraSpacePoint;

	Plane planeOfTriangle;
	planeOfTriangle.SetFromTriangle(triangle);

	for (int row = minRow; row <= maxRow; row++)
	{
		Edge* edges = (row < midRow) ? minEdges : maxEdges;
		
		double y = double(row);
		
		double d0 = edges[0].vertexA->y - edges[0].vertexB->y;
		if (d0 == 0.0)
			continue;

		double t0 = (y - edges[0].vertexB->y) / d0;
		if (isinf(t0) || isnan(t0))
			continue;

		double d1 = edges[1].vertexA->y - edges[1].vertexB->y;
		if (d1 == 0.0)
			continue;

		double t1 = (y - edges[1].vertexB->y) / d1;
		if (isinf(t1) || isnan(t1))
			continue;

		t0 = FRUMPY_CLAMP(t0, 0.0, 1.0);
		t1 = FRUMPY_CLAMP(t1, 0.0, 1.0);

		double x0 = edges[0].vertexA->x * t0 + edges[0].vertexB->x * (1.0 - t0);
		double x1 = edges[1].vertexA->x * t1 + edges[1].vertexB->x * (1.0 - t1);
		
		double minX = FRUMPY_MIN(x0, x1);
		double maxX = FRUMPY_MAX(x0, x1);
		
		int minCol = int(floor(minX));
		int maxCol = int(ceil(maxX));

		minCol = FRUMPY_MAX(minCol, 0);
		maxCol = FRUMPY_MIN(maxCol, signed(this->width));

		for (int col = minCol; col <= maxCol; col++)
		{
			Location location{ unsigned(row), unsigned(col) };

			Vector imagePointA(double(col), double(row), 0.0);
			Vector imagePointB(double(col), double(row), -1.0);
			
			Vector cameraPointA, cameraPointB;
			
			pipelineMatrices.imageToCamera.TransformPoint(imagePointA, cameraPointA);
			pipelineMatrices.imageToCamera.TransformPoint(imagePointB, cameraPointB);
			
			Vector rayDirection = cameraPointB - cameraPointA;
			double lambda = 0.0;
			planeOfTriangle.RayCast(cameraPointA, rayDirection, lambda);
			Vector trianglePoint = cameraPointA + rayDirection * lambda;

			Pixel* pixelZ = depthBuffer.GetPixel(location);
			if (pixelZ && trianglePoint.z < pixelZ->depth)	// TODO: Depth-testing optional?
			{
				pixelZ->depth = (float)trianglePoint.z;
				Pixel* pixel = this->GetPixel(location);

				Vector baryCoords;
				triangle.CalcBarycentricCoordinates(trianglePoint, baryCoords);

				Vector interpolatedColor =
					vertexA.color * baryCoords.x +
					vertexB.color * baryCoords.y +
					vertexC.color * baryCoords.z;

				Vector interpolatedTexCoords =
					vertexA.texCoords * baryCoords.x +
					vertexB.texCoords * baryCoords.y +
					vertexC.texCoords * baryCoords.z;	// TODO: Do something with this.  Will it looks correct?

				pixel->color.SetColor(
					unsigned char(interpolatedColor.x * 255.0),
					unsigned char(interpolatedColor.y * 255.0),
					unsigned char(interpolatedColor.z * 255.0),
					0);
			}
		}
	}
}