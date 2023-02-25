#pragma once

#include "Defines.h"
#include "Math/Matrix4x4.h"
#include "Containers/List.h"
#include "Math/Plane.h"
#include "Renderer.h"
#include <functional>

namespace Frumpy
{
	class Renderer;
	class Camera;
	class ConvexHull;

	// This defines the scene as a hierarchy of scene objects.
	class FRUMPY_API Scene
	{
	public:
		Scene();
		virtual ~Scene();

		class Object;
		typedef List<Object*> ObjectList;

		Object* FindObjectByName(const char* name);
		void ForAllObjects(std::function<bool(Object*)> lambda);
		void GenerateVisibleObjectsList(const Camera* camera, ObjectList& visibleObjectList, const Matrix4x4& worldToCamera) const;

		// Derivatives of this are anything that might get rendered in the scene.
		class FRUMPY_API Object
		{
		public:
			Object();
			virtual ~Object();

			virtual void CalculateWorldTransform(const Matrix4x4& parentToWorld) const;
			virtual bool IntersectsFrustum(const ConvexHull& frustumHull, const Matrix4x4& worldToCamera) const;
			virtual void Render(Renderer& renderer, const Camera* camera) const;

			void SetRenderFlag(uint32_t renderFlag, bool enabled);
			bool GetRenderFlag(uint32_t renderFlag) const;

			void SetName(const char* givenName);
			const char* GetName() const { return this->name; }

			Matrix4x4 childToParent;
			mutable Matrix4x4 objectToWorld;
			ObjectList childObjectList;
			uint32_t renderFlags;

		protected:
			char name[128];
			mutable std::vector<Renderer::Vertex>* renderVertexBuffer;
		};

		ObjectList objectList;
	};
}