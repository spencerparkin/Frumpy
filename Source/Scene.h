#pragma once

#include "Defines.h"
#include "Matrix.h"
#include "List.h"
#include "Plane.h"
#include <functional>

namespace Frumpy
{
	class Renderer;
	class Camera;

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
		void GenerateVisibleObjectsList(const Camera* camera, ObjectList& visibleObjectList) const;

		// Derivatives of this are anything that might get rendered in the scene.
		class FRUMPY_API Object
		{
		public:
			Object();
			virtual ~Object();

			virtual void CalculateWorldTransform(const Matrix& parentToWorld) const;
			virtual bool IntersectsFrustum(const List<Plane>& frustumPlanesList) const;
			virtual void Render(Renderer& renderer) const;

			enum RenderType
			{
				RENDER_OPAQUE,
				RENDER_TRANSPARENT
			};

			char name[128];
			Matrix childToParent;
			mutable Matrix objectToWorld;
			ObjectList childObjectList;
			RenderType renderType;
			bool castsShadow;
			bool canBeShadowed;
		};

		ObjectList objectList;
	};
}