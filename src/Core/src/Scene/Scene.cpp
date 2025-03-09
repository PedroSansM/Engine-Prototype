#include "Scene.h"
#include "AssetManagerTypes.h"
#include "ComponentId.h"
#include "SerializationTypes.h"
#include "UUIDComponent.h"
#include "RootComponent.h"
#include "NameComponent.h"
#include "ComponentForm.h"
#include "TransformComponent.h"
#include "TemplateUtils.h"
#include "UUID.h"
#include "ReadWriteLockGuard.h"
#include "Asset.h"
#include "ChildComponent.h"
#include "ChildrenComponent.h"
#include "ComponentRefSpecialization.h"



namespace DCore
{

// Begin Scene
Scene::Scene()
	:
	m_loaded(false)
{}

Scene::Scene(const Scene& other)
	:
	m_registry(other.m_registry),
	m_name(other.m_name),
	m_loaded(other.m_loaded)
{}

Scene::Scene(const DString& name)
	:
	m_name(name),
	m_loaded(false)
{}

Scene::Scene(Scene&& other) noexcept
	:
	m_registry(std::move(other.m_registry)),
	m_name(std::move(other.m_name)),
	m_loaded(other.m_loaded)
{}

Entity Scene::CreateEntity(const stringType& entityName)
{
	constexpr size_t numberOfComponents{4};
	const ComponentIdType componentIds[numberOfComponents]
	{
		ComponentId::GetId<NameComponent>(),
		ComponentId::GetId<UUIDComponent>(),
		ComponentId::GetId<TransformComponent>(),
		ComponentId::GetId<RootComponent>()
	};
	const size_t componentSizes[numberOfComponents]
	{
		sizeof(NameComponent),
		sizeof(UUIDComponent),
		sizeof(TransformComponent),
		sizeof(RootComponent)
	};
	Entity entity
	(
		m_registry.CreateEntity
		(
			componentIds, componentSizes, numberOfComponents,
			[&](ComponentIdType componentId, void* component) -> bool
			{
				if (componentId == ComponentId::GetId<NameComponent>())
				{
					ConstructorArgs<NameComponent> args;
					args.Name = entityName;
					new (component) NameComponent(args);
					return false;
				}
				if (componentId == ComponentId::GetId<UUIDComponent>())
				{
					ConstructorArgs<UUIDComponent> args;
					UUIDGenerator::Get().GenerateUUID(args.UUID);
					new (component) UUIDComponent(args);
					return false;
				}
				if (componentId == ComponentId::GetId<TransformComponent>())
				{
					const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
					new (component) TransformComponent(*static_cast<const ConstructorArgs<TransformComponent>*>(componentForm.DefaultArgs));
					return false;
				}
				if (componentId == ComponentId::GetId<RootComponent>())
				{
					new (component) RootComponent();
					return false;
				}
				DASSERT_E(false);
				return false;
			}
		)
	);
	return entity;
}

void Scene::Clear()
{
//m_registry.Clear();
	m_name.Clear();
}
// End Scene

// Begin SceneRef
SceneRef::SceneRef()
	:
	m_lockData(nullptr)
{}

SceneRef::SceneRef(InternalSceneRefType ref, LockData& lockData)
	:
	m_ref(ref),
	m_lockData(&lockData)
{}

SceneRef::SceneRef(const SceneRef& other)
	:
	m_ref(other.m_ref),
	m_lockData(other.m_lockData)
{}

bool SceneRef::IsValid() const
{
	if (m_lockData == nullptr)
	{
		return false;
	}
	return m_ref.IsValid();
}

void SceneRef::GetUUID(UUIDType& outUUID) const
{
	DASSERT_E(IsValid());
	outUUID = m_ref->GetUUID();
}

void SceneRef::SetName(const DString& name)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	m_ref->GetAsset().SetName(name);
}

Entity SceneRef::CreateEntity(const stringType& entityName)
{
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	DASSERT_E(IsValid());
	return m_ref->GetAsset().CreateEntity(entityName); 
}

void SceneRef::GetName(DString& outName) const
{
	DASSERT_E(IsValid());
	outName = m_ref->GetAsset().GetName().Data();
}
// End SceneRef

}
