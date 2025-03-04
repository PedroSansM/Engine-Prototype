#pragma once

#include "DCoreAssert.h"
#include "ComponentId.h"
#include "ReadWriteLockGuard.h"
#include "Registry.h"
#include "SerializationTypes.h"
#include "ComponentForm.h"
#include "ECSTypes.h"
#include "SceneTypes.h"
#include "TemplateUtils.h"
#include "UUID.h"
#include "Asset.h"
#include "ComponentRef.h"
#include "Component.h"
#include "AssetManagerTypes.h"

#include <string>
#include <tuple>
#include <vector>




namespace DCore
{

class ChildrenComponent;

class Scene
{
public:
	using stringType = std::string;
public:
	Scene();
	Scene(const Scene&);
	Scene(const DString&);
	Scene(Scene&&) noexcept;
	~Scene() = default;
public:
	Entity CreateEntity(const stringType& entityName);
	void Clear();
public:
	const DString& GetName() const
	{
		return m_name;
	}

	void SetName(const DString& name)
	{
		m_name = name;
	}

	Registry& GetRegistry()
	{
		return m_registry;
	}

	const Registry& GetRegistry() const
	{
		return m_registry;
	}

	void LoadingCompleted()
	{
		m_loaded = true;
	}

	bool IsLoaded() const
	{
		return m_loaded;
	}
private:
	Registry m_registry;
	DString m_name;
	bool m_loaded; // Only after m_loaded is true, Runtime should tick this scene.
};

using InternalSceneRefType = typename AssetContainerType<Scene, SceneIdType, SceneVersionType>::Ref;

class SceneRef
{
public:
	using stringType = Scene::stringType;
public:
	SceneRef();
	SceneRef(InternalSceneRefType ref, LockData&);
	SceneRef(const SceneRef&);
	~SceneRef() = default;
public:
	bool IsValid() const;
	void GetUUID(UUIDType& outUUID) const;
	void SetName(const DString& name);
	Entity CreateEntity(const stringType& entityName);
	void GetName(DString& outName) const;
public:
	InternalSceneRefType GetInternalSceneRef()
	{
		return m_ref;
	}

	SceneIdType GetInternalSceneRefId()
	{
		return m_ref.GetId();
	}

	SceneVersionType GetInternalSceneRefVersion()
	{
		return m_ref.GetVersion();
	}

	LockData* GetLockData()
	{
		return m_lockData;
	}

	Entity GetEntityWithIdAndVersion(EntityIdType id, EntityVersionType version)
	{
		DASSERT_E(IsValid());
		return m_ref->GetAsset().GetRegistry().GetEntityWithIdAndVersion(id, version);
	}

	void LoadingCompleted()
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		m_ref->GetAsset().LoadingCompleted();
	}

	bool IsLoaded() const
	{
		DASSERT_E(IsValid());
		return m_ref->GetAsset().IsLoaded();
	}
public:
	SceneRef& operator=(const SceneRef& other)
	{
		m_ref = other.m_ref;
		m_lockData = other.m_lockData;
		return *this;
	}

	bool operator==(const SceneRef& other) const
	{
		DASSERT_E(IsValid());
		return m_ref->GetUUID() == other.m_ref->GetUUID();
	}
public:
	template <class Func>
	Entity CreateEntity(const ComponentId* componentIds, const size_t* componentSizes, size_t numberOfComponents, Func function)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		return m_ref->GetAsset().GetRegistry().CreateEntity(componentIds, componentSizes, numberOfComponents, function);
	}

	template <class Component, class ...Components, class TupleArg, class ...TupleArgs>
	Entity CreateEntity(TupleArg&& tupleArg, TupleArgs&& ...tupleArgs)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		return m_ref->GetAsset().GetRegistry().CreateEntity<Component, Components...>(std::forward<TupleArg>(tupleArg), std::forward<TupleArgs>(tupleArgs)...);
	}

	template <class Component, class ...Components, class Func>
	void Iterate(Func function)
	{
		DASSERT_E(IsValid());
		m_ref->GetAsset().GetRegistry().Iterate<Component, Components...>
		(
			[&](Entity entity, Component&, Components&...) -> bool
			{
				return std::invoke(function, entity, ComponentRef<Component>(entity, m_ref, *m_lockData), ComponentRef<Components>(entity, m_ref, *m_lockData)...);
			}
		);
	}

	template <class Func>
	void Iterate(const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(IsValid());
		m_ref->GetAsset().GetRegistry().Iterate
		(
			componentIds, numberOfComponents,
			[&](Entity entity, ComponentIdType componentId, void*) -> bool
			{
				return std::invoke(function, entity, std::make_tuple(entity, m_ref, componentId, m_lockData));
			}	
		);
	}

	template <class Func>
	void IterateOnEntities(Func function)
	{
		DASSERT_E(IsValid());
		m_ref->GetAsset().GetRegistry().IterateOnEntities
		(
			[&](Entity entity) -> bool
			{
				return std::invoke(function, entity);
			}
		);
	}

	template <class Func>
	void IterateOnEntities(Func function) const
	{
		DASSERT_E(IsValid());
		m_ref->GetAsset().GetRegistry().IterateOnEntities
		(
			[&](Entity entity) -> bool
			{
				return std::invoke(function, entity);
			}
		);
	}

	template <class Component, class ...Components>
	size_t GetNumberOfEntitiesWithComponents() const
	{
		DASSERT_E(IsValid());
		const Registry& registry(m_ref->GetAsset().GetRegistry());
		size_t result(0);
		registry.Iterate<Component, Components...>
		(
			[&](Entity, const Component&, const Components&...) -> bool
			{
				result++;
				return false;
			}
		);
		return result;
	}
private:
	InternalSceneRefType m_ref;
	LockData* m_lockData;
};

}
