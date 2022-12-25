#pragma once

#include "../Scene.h"

namespace Frumpy
{
	class Mesh;

	class FRUMPY_API MeshObject : public Scene::Object
	{
	public:
		MeshObject();
		virtual ~MeshObject();

		virtual bool IntersectsFrustum(const List<Plane>& frustumPlanesList) const override;
		virtual void Render(Renderer& renderer) const override;

		void SetMesh(Mesh* mesh) { this->mesh = mesh; }
		Mesh* GetMesh() { return this->mesh; }

	private:
		Mesh* mesh;
		// TODO: Also have pointer to material asset?
	};
}