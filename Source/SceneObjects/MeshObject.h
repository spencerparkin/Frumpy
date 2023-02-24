#pragma once

#include "Scene.h"
#include "FileAssets/Image.h"
#include "Math/ConvexHull.h"

namespace Frumpy
{
	class Mesh;
	class Image;
	class Camera;
	struct Edge;

	class FRUMPY_API MeshObject : public Scene::Object
	{
	public:
		MeshObject();
		virtual ~MeshObject();

		virtual bool IntersectsFrustum(const ConvexHull& frustumHull, const Matrix4x4& worldToCamera) const override;
		virtual void Render(Renderer& renderer, const Camera* camera) const override;

		void SetMesh(Mesh* mesh) { this->mesh = mesh; }
		Mesh* GetMesh() { return this->mesh; }

		void SetTexture(Image* texture) { this->texture = texture; }
		Image* GetTexture() { return this->texture; }

		void SetSampleMethod(Image::SampleMethod sampleMethod) { this->sampleMethod = sampleMethod; }
		Image::SampleMethod GetSampleMethod() const { return this->sampleMethod; }

		void SetColor(const Vector4& color);

	private:

		Mesh* mesh;
		Image* texture;
		Image::SampleMethod sampleMethod;

		mutable ConvexHull objectSpaceBoundingHull;
		mutable bool objectSpaceBoundingHullValid;
	};
}