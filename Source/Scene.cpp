#include "Scene.h"
#include "Camera.h"
#include "Renderer.h"
#include "FileAssets/Image.h"

using namespace Frumpy;

Scene::Scene()
{
}

/*virtual*/ Scene::~Scene()
{
	this->objectList.Delete();
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

void Scene::GenerateVisibleObjectsList(const Camera* camera, ObjectList& visibleObjectList, const Matrix4x4& worldToCamera) const
{
	// Go determine the list of objects visible to the viewing frustum of the given camera.  This, of
	// course, does not account for any kind of occlusion that may be taking place.  The depth buffer will
	// take care of any occlusion, so we're not able to necessarily skip anything fully occluded here.
	// Note that in the future, some sort of spatial partitioning data-structure may be helpful here, such
	// as a binary-space partition tree.
	const ConvexHull& frustumHull = camera->frustum.GetFrustumHull();
	const_cast<Scene*>(this)->ForAllObjects([&visibleObjectList, &frustumHull, &worldToCamera](Object* object) -> bool {
		if (object->GetRenderFlag(Object::VISIBLE) && object->IntersectsFrustum(frustumHull, worldToCamera))
			visibleObjectList.AddTail(object);
		return true;
	});
}

Scene::Object::Object()
{
	this->name[0] = '\0';
	this->renderFlags = RenderFlag::VISIBLE | RenderFlag::IS_LIT;
}

/*virtual*/ Scene::Object::~Object()
{
	this->childObjectList.Delete();
}

void Scene::Object::SetRenderFlag(RenderFlag renderFlag, bool enabled)
{
	uint32_t flagBit = (uint32_t)renderFlag;
	if (enabled)
		this->renderFlags |= flagBit;
	else
		this->renderFlags &= ~flagBit;
}

bool Scene::Object::GetRenderFlag(RenderFlag renderFlag) const
{
	uint32_t flagBit = (uint32_t)renderFlag;
	return (this->renderFlags & flagBit) != 0;
}

/*virtual*/ void Scene::Object::CalculateWorldTransform(const Matrix4x4& parentToWorld) const
{
	this->objectToWorld = parentToWorld * this->childToParent;

	for (const ObjectList::Node* node = this->childObjectList.GetHead(); node; node = node->GetNext())
	{
		const Object* childObject = node->value;
		childObject->CalculateWorldTransform(this->objectToWorld);
	}
}

/*virtual*/ void Scene::Object::Render(Renderer& renderer, const Camera* camera) const
{
	// This method must be overridden for anything to actually get rendered.
}

/*virtual*/ bool Scene::Object::IntersectsFrustum(const ConvexHull& frustumHull, const Matrix4x4& worldToCamera) const
{
	// Deriviative classes should override this function as well.
	return true;
}