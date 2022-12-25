#include "Scene.h"
#include "Camera.h"
#include "Renderer.h"
#include "FileAssets/Image.h"

using namespace Frumpy;

Scene::Scene()
{
	this->clearPixel.color = 0;
}

/*virtual*/ Scene::~Scene()
{
	this->objectList.Delete();
}

/*virtual*/ void Scene::Render(const Camera& camera, Renderer& renderer) const
{
	// First, resolve the object-to-world transform for each object in the scene hierarchy.
	Matrix parentToWorld;
	parentToWorld.Identity();
	for (const ObjectList::Node* node = this->objectList.GetHead(); node; node = node->GetNext())
	{
		const Object* object = node->value;
		object->CalculateWorldTransform(parentToWorld);
	}

	// Calculate the other matrices we'll need.
	camera.frustum.CalcProjectionMatrix(renderer.pipelineMatrices.cameraToProjection);
	renderer.pipelineMatrices.worldToCamera.Invert(camera.worldTransform);
	renderer.GetImage()->CalcImageMatrix(renderer.pipelineMatrices.projectionToImage);
	renderer.pipelineMatrices.worldToImage =
		renderer.pipelineMatrices.projectionToImage *
		renderer.pipelineMatrices.cameraToProjection *
		renderer.pipelineMatrices.worldToCamera;
	renderer.pipelineMatrices.imageToCamera.Invert(
		renderer.pipelineMatrices.projectionToImage *
		renderer.pipelineMatrices.cameraToProjection);

	// Next, go determine the list of objects visible to the viewing frustum.  This, of course,
	// does not account for any kind of occlusion that may be taking place.  The depth buffer will
	// take care of any occlusion, but we're not able to necessarily skip anything fully occluded here.
	List<Plane> frustumPlanesList;
	camera.frustum.GeneratePlanes(frustumPlanesList);
	ObjectList visibleObjectList;
	const_cast<Scene*>(this)->ForAllObjects([&visibleObjectList, &frustumPlanesList](Object* object) -> bool {
		if (object->IntersectsFrustum(frustumPlanesList))
			visibleObjectList.AddTail(object);
		return true;
	});

	// Clear the depth buffer before we start using it.
	Image::Pixel pixel;
	pixel.depth = -(float)camera.frustum._far;
	renderer.GetDepthBuffer()->Clear(pixel);

	// Clear the image buffer before we start rasterizing to it.
	renderer.GetImage()->Clear(this->clearPixel);

	// Finally, go render the list of objects we think are visible.
	for (ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
	{
		const Object* object = node->value;
		object->Render(renderer);
	}

	// We're done rendering when all render jobs complete.
	renderer.WaitForAllJobCompletion();
}

Scene::Object* Scene::FindObjectByName(const char* name)
{
	Object* foundObject = nullptr;
	this->ForAllObjects([name, &foundObject](Object* object) -> bool {
		if (0 == strcmp(name, object->name))
		{
			foundObject = object;
			return false;
		}
		return true;
	});
	return foundObject;
}

void Scene::ForAllObjects(std::function<bool(Object*)> lambda)
{
	ObjectList objectQueue;
	for (ObjectList::Node* node = this->objectList.GetHead(); node; node = node->GetNext())
		objectQueue.AddTail(node->value);

	while (objectQueue.GetCount())
	{
		ObjectList::Node* node = objectQueue.GetHead();
		Object* object = node->value;
		objectQueue.Remove(node);
		if (!lambda(object))
			break;

		for (ObjectList::Node* node = object->childObjectList.GetHead(); node; node = node->GetNext())
			objectQueue.AddTail(node->value);
	}
}

Scene::Object::Object()
{
	this->name[0] = '\0';
}

/*virtual*/ Scene::Object::~Object()
{
	this->childObjectList.Delete();
}

/*virtual*/ void Scene::Object::CalculateWorldTransform(const Matrix& parentToWorld) const
{
	this->objectToWorld = parentToWorld * this->childToParent;

	for (const ObjectList::Node* node = this->childObjectList.GetHead(); node; node = node->GetNext())
	{
		const Object* childObject = node->value;
		childObject->CalculateWorldTransform(this->objectToWorld);
	}
}

/*virtual*/ void Scene::Object::Render(Renderer& renderer) const
{
	// This method must be overridden for anything to actually get rendered.
}

/*virtual*/ bool Scene::Object::IntersectsFrustum(const List<Plane>& frustumPlanesList) const
{
	// Deriviative classes should override this function as well.
	return true;
}