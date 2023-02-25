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
		if (0 == strcmp(name, object->GetName()))
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
		if (object->GetRenderFlag(FRUMPY_RENDER_FLAG_VISIBLE) && object->IntersectsFrustum(frustumHull, worldToCamera))
			visibleObjectList.AddTail(object);
		return true;
	});
}

Scene::Object::Object()
{
	this->name[0] = '\0';
	this->renderFlags = FRUMPY_RENDER_FLAG_VISIBLE | FRUMPY_RENDER_FLAG_IS_LIT | FRUMPY_RENDER_FLAG_DEPTH_TEST | FRUMPY_RENDER_FLAG_BACK_FACE_CULL;
}

/*virtual*/ Scene::Object::~Object()
{
	this->childObjectList.Delete();
}

void Scene::Object::SetName(const char* givenName)
{
	strcpy_s(this->name, sizeof(this->name), givenName);
}

void Scene::Object::SetRenderFlag(uint32_t renderFlag, bool enabled)
{
	if (enabled)
		this->renderFlags |= renderFlag;
	else
		this->renderFlags &= ~renderFlag;
}

bool Scene::Object::GetRenderFlag(uint32_t renderFlag) const
{
	return (this->renderFlags & renderFlag) != 0;
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