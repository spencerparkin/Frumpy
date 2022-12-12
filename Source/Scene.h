#pragma once

#include "Defines.h"
#include "Matrix.h"
#include "List.h"
#include "Camera.h"
#include "Image.h"
#include <functional>

namespace Frumpy
{
	class Image;

	// This defines the scene as a hierarchy of scene objects.
	class FRUMPY_API Scene
	{
	public:
		Scene();
		virtual ~Scene();

		// This is it!  This is the main entry-point into Frumpy.  Frumpy is just
		// a bare-bones, simple-as-possible, software renderer.  That's all it is.
		// That's all it does.  You want fancy-pants to the max?  Use DX12 or Volkan.
		virtual void Render(const Camera& camera, Image& image) const;

		class Object;
		typedef List<Object*> ObjectList;

		// This is anything that might get rendered in the scene.
		class FRUMPY_API Object
		{
		public:
			Object();
			virtual ~Object();

			virtual void CalculateWorldTransform(const Matrix& parentMatrix) const;
			virtual bool IntersectsFrustum(const List<Plane>& frustumPlanesList) const;
			virtual void Render(const Matrix& cameraMatrix, const Matrix& projectionMatrix, Image& image, Image& depthBuffer) const;

			Matrix localTransform;
			mutable Matrix worldTransform;
			ObjectList childObjectList;
		};

		void ForAllObjects(std::function<bool(Object*)> lambda);

		ObjectList objectList;
		Image::Pixel clearPixel;
		// TODO: Add list of lights?  How would we cast shadows?

	protected:
		mutable Image depthBuffer;
	};
}