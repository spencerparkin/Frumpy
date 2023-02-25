#include "Shader.h"
#include "Renderer.h"

using namespace Frumpy;

VertexShader::VertexShader(const Matrix4x4& objectToWorld, const GraphicsMatrices& graphicsMatrices)
{
	this->objectToImage = graphicsMatrices.worldToImage * objectToWorld;
	this->objectToCamera = graphicsMatrices.worldToCamera * objectToWorld;
}

/*virtual*/ VertexShader::~VertexShader()
{
}

PixelShader::PixelShader()
{
}

/*virtual*/ PixelShader::~PixelShader()
{
}