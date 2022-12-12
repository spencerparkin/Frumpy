#include "Scene.h"
#include "Camera.h"

using namespace Frumpy;

Scene::Scene()
{
	//this->clearPixel.color.SetColor(0.0, 0.0, 0.0, 0.0);
}

/*virtual*/ Scene::~Scene()
{
	this->objectList.Delete();
}

/*virtual*/ void Scene::Render(const Camera& camera, Image& image) const
{
	// First, resolve the world transformation for each object in the scene hierarchy.
	Matrix parentMatrix;
	parentMatrix.Identity();
	for (const ObjectList::Node* node = this->objectList.GetHead(); node; node = node->GetNext())
	{
		const Object* object = node->value;
		object->CalculateWorldTransform(parentMatrix);
	}

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

	// Before we can go render, we need a depth buffer.  Make sure we have one now.
	this->depthBuffer.SetWidthAndHeight(image.GetWidth(), image.GetHeight());
	Image::Pixel pixel;
	pixel.depth = (float)camera.frustum.far;
	this->depthBuffer.Clear(pixel);

	// Also, clear the given image buffer before we start rasterizing to it.
	image.Clear(this->clearPixel);

	// Finally, go render the list of objects we think are visible.
	Matrix projectionMatrix;
	camera.frustum.CalcProjectionMatrix(projectionMatrix);
	Matrix cameraMatrix;
	cameraMatrix.Invert(camera.worldTransform);
	for (ObjectList::Node* node = visibleObjectList.GetHead(); node; node = node->GetNext())
	{
		const Object* object = node->value;
		object->Render(cameraMatrix, projectionMatrix, image, depthBuffer);
	}
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
		if (!lambda(object))
			break;

		for (ObjectList::Node* node = object->childObjectList.GetHead(); node; node = node->GetNext())
			objectQueue.AddTail(node->value);
	}
}

Scene::Object::Object()
{
}

/*virtual*/ Scene::Object::~Object()
{
	this->childObjectList.Delete();
}

/*virtual*/ void Scene::Object::CalculateWorldTransform(const Matrix& parentMatrix) const
{
	this->worldTransform.Multiply(parentMatrix, this->localTransform);

	for (const ObjectList::Node* node = this->childObjectList.GetHead(); node; node = node->GetNext())
	{
		const Object* childObject = node->value;
		childObject->CalculateWorldTransform(this->worldTransform);
	}
}

/*virtual*/ void Scene::Object::Render(const Matrix& cameraMatrix, const Matrix& projectionMatrix, Image& image, Image& depthBuffer) const
{
	// This method must be overridden for anything to actually get rendered.
}

/*virtual*/ bool Scene::Object::IntersectsFrustum(const List<Plane>& frustumPlanesList) const
{
	// Deriviative classes should override this function as well.
	return true;
}