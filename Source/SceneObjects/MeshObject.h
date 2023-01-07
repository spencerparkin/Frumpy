#pragma once

#include "../Scene.h"
#include "../FileAssets/Image.h"

namespace Frumpy
{
	class Mesh;
	class Image;

	class FRUMPY_API MeshObject : public Scene::Object
	{
	public:
		MeshObject();
		virtual ~MeshObject();

		virtual bool IntersectsFrustum(const List<Plane>& frustumPlanesList) const override;
		virtual void Render(Renderer& renderer) const override;

		void SetMesh(Mesh* mesh) { this->mesh = mesh; }
		Mesh* GetMesh() { return this->mesh; }

		void SetTexture(Image* texture) { this->texture = texture; }
		Image* GetTexture() { return this->texture; }

		void SetSampleMethod(Image::SampleMethod sampleMethod) { this->sampleMethod = sampleMethod; }
		Image::SampleMethod GetSampleMethod() const { return this->sampleMethod; }

	private:
		Mesh* mesh;
		Image* texture;
		Image::SampleMethod sampleMethod;
	};
}