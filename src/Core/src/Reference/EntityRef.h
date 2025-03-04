#pragma once

#include "DCoreAssert.h"
#include "ECSTypes.h"
#include "SerializationTypes.h"
#include "UUID.h"
#include "Scene.h"
#include "AssetManagerTypes.h"
#include "Asset.h"
#include "ComponentRef.h"
#include "ComponentRefSpecialization.h"
#include "ReadWriteLockGuard.h"

#include <vector>



namespace DCore
{

class ChildrenComponent;
class ChildComponent;

/// To use this API, SceneAssetManager must be locked for (and only for) reading.
class EntityRef
{
	friend class SceneSerialization;
	friend class SceneManager;
public:
	using entityContainerType = std::vector<EntityRef>;
public:
	EntityRef();
	EntityRef(const EntityRef&);
	EntityRef(Entity, SceneRef);
	~EntityRef() = default; 
public:
	bool IsValid() const;
	void SetParent(EntityRef);
	bool TryGetParent(EntityRef&);
	bool HaveParent() const;
	size_t GetNumberOfChildren() const;
	void RemoveChild(EntityRef, bool toRemoveChildComponent = true);
	void GetName(DString& outName) const;
	void SetName(const DString&);
	void GetUUID(UUIDType& out) const;
	void Destroy();
	void RemoveParent();
	bool HaveChild(EntityRef) const;
	bool HaveChildRecursive(EntityRef) const;
	bool HaveChildren() const;
	entityContainerType GetChildren() const; // Slow
	ComponentRef<Component> GetComponent(DCore::ComponentIdType);
	const ComponentRef<Component> GetComponent(DCore::ComponentIdType) const;
	bool HaveComponent(DCore::ComponentIdType) const;
	bool HaveComponents(const DCore::ComponentIdType* componentIds, size_t numberOfComponents) const;
	void RemoveComponents(const ComponentIdType* componentIds, size_t numberOfComponents);
	void GetSceneUUID(UUIDType& outUUID) const;
	size_t GetNumberOfComponents() const;
	void GetComponentIds(ComponentIdType* outComponentIds);
	DMat4 GetWorldModelMatrix() const;
	DVec3 GetWorldTranslation() const;
	void SetWorldTranslation(const DVec3&);
	DVec3 GetLocalTranslation() const;
	void SetLocalTranslation(const DVec3&);
	void RemoveParentTransformationsFrom(DMat4&) const;
	EntityRef Duplicate();
public:
	Entity GetEntity() const
	{
		return m_entity;
	}

	InternalSceneRefType GetInternalSceneRef() const
	{
		return m_internalSceneRef;
	}

	LockData* GetLockData() const
	{
		return m_lockData;
	}

	SceneRef GetSceneRef() const
	{
		return SceneRef(m_internalSceneRef, *m_lockData);
	}

	void Invalidate()
	{
		m_entity.Invalidate();
		m_lockData = nullptr;
	}
public:
	void operator=(const EntityRef& other)
	{
		m_entity = other.m_entity;
		m_internalSceneRef = other.m_internalSceneRef;
		m_lockData = other.m_lockData;
	}

	bool operator==(const EntityRef& other) const
	{
		return m_entity == other.m_entity && m_internalSceneRef->GetUUID() == other.m_internalSceneRef->GetUUID();
	}

	bool operator!=(const EntityRef& other)
	{
		return !operator==(other);
	}
public:
	template <class Func>
	void AddComponents(const ComponentIdType* componentIds, const size_t* componentSizes, size_t numberOfComponents, Func function)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		m_internalSceneRef->GetAsset().GetRegistry().AddComponents
		(
			m_entity, componentIds, componentSizes, numberOfComponents,
			[&](ComponentIdType componentId, void* componentAddress) -> void
			{
				std::invoke(function, componentId, componentAddress);
			}
		);
	}

	template <class Component, class ...Components, class TupleArg, class ...TupleArgs>
	void AddComponents(TupleArg&& arg, TupleArgs&&... args)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		m_internalSceneRef->GetAsset().GetRegistry().AddComponents<Component, Components...>(m_entity, std::forward<TupleArg>(arg), std::forward<TupleArgs>(args)...);
	}
	
	template <class Component, class ...Components>
	decltype(auto) GetComponents()
	{
		DASSERT_E(IsValid());
		const bool haveComponents(m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<Component, Components...>(m_entity));
		DASSERT_E(haveComponents);
		if constexpr (DCore::TypeList<Component, Components...>::size == 1)
		{
			return ComponentRef<Component>(m_entity, m_internalSceneRef, *m_lockData);
		}
		else
		{
			return std::make_tuple(ComponentRef<Component>(m_entity, m_internalSceneRef, *m_lockData), ComponentRef<Components>(m_entity, m_internalSceneRef, *m_lockData)...);
		}
	}	

	template <class Component, class ...Components>
	decltype(auto) GetComponents() const
	{
		DASSERT_E(IsValid());
		bool haveComponents(m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<Component, Components...>(m_entity));
		DASSERT_E(haveComponents);
		if constexpr (DCore::TypeList<Component, Components...>::size == 1)
		{
			return ComponentRef<Component>(m_entity, m_internalSceneRef, *m_lockData);
		}
		else
		{
			return std::make_tuple(ComponentRef<Component>(m_entity, m_internalSceneRef, *m_lockData), ComponentRef<Components>(m_entity, m_internalSceneRef, *m_lockData)...);
		}
	}	

	template <class Func>
	void GetComponents(const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(IsValid() && HaveComponents(componentIds, numberOfComponents));
		for (size_t i(0); i < numberOfComponents; i++)
		{
			std::invoke(function, ComponentRef<Component>(m_entity, m_internalSceneRef, componentIds[i], *m_lockData));
		}
	}

	template <class Component, class ...Components, class OutValue>
	bool TryGetComponents(OutValue& out)
	{
		DASSERT_E(IsValid());
		Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
		if (!registry.HaveComponents<Component, Components...>(m_entity))
		{
			return false;;
		}
		if constexpr (TypeList<Component, Components...>::size == 1)
		{
			out = ComponentRef<Component>(m_entity, m_internalSceneRef, *m_lockData);
		}
		else
		{
			out = std::make_tuple(ComponentRef<Component>(m_entity, m_internalSceneRef, *m_lockData), ComponentRef<Components>(m_entity, m_internalSceneRef, *m_lockData)...);
		}
		return true;
	}
	 
	template <class Component, class ...Components>
	void RemoveComponents()
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		m_internalSceneRef->GetAsset().GetRegistry().RemoveComponents<Component, Components...>(m_entity);
	}

	template <class Component, class ...Components>
	bool HaveComponents()
	{
		DASSERT_E(IsValid());
		return m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<Component, Components...>(m_entity);
	}

	template <class Func>
	void IterateOnChildren(Func function)
	{
		DASSERT_E(IsValid());
		DASSERT_E(m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<ChildrenComponent>(m_entity));
		ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		IterateOnChildren(function, childrenComponent);
	}

	template <class Func>
	void IterateOnParents(Func function)
	{
		DASSERT_E(IsValid());
		if (!m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<ChildComponent>(m_entity))
		{
			return;
		}	
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		IterateOnParents(function, childComponent);
	}

	template <class Func>
	void IterateOnParents(Func function) const
	{
		DASSERT_E(IsValid());
		if (!m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<ChildComponent>(m_entity))
		{
			return;
		}
		const ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		IterateOnParents(function, childComponent);
	}

	template <class Func>
	void IterateOnComponents(Func function)
	{
		if (!IsValid())
		{
			return;
		}
		m_internalSceneRef->GetAsset().GetRegistry().IterateOnComponents(m_entity, function);
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
private:
	static EntityRef Duplicate(EntityRef entityToDuplicate, EntityRef parent);
private:
	template <class AssetRefType, class GetRefFunc>
	static void DuplicateAssetAndAssignItToComponent(
		AttributeIdType attributeId,
		AssetRefType assetRef,
		AttributeType attributeType,
		Component* component,
		GetRefFunc getRefFunction)
	{
		if (!assetRef.IsValid())
		{
			return;
		}
		AssetRefType duplicatedAsset(std::invoke(getRefFunction, assetRef.GetUUID()));
		component->OnAttributeChange(attributeId, &duplicatedAsset, attributeType);
	}
private:
	template <class Func, class ChildrenComponentT>
	void IterateOnChildren(Func function, ChildrenComponentT& childrenComponent)
	{
		static_assert(std::is_same_v<ChildrenComponentT, ChildrenComponent>);
		ChildrenComponentT::IterateOnChildren(function, childrenComponent);
	}

	template <class Func, class ChildComponentT>
	void IterateOnParents(Func function, ChildComponentT& childComponent)
	{
		static_assert(std::is_same_v<ChildComponentT, ChildComponent>);
		ChildComponentT::IterateOnParents(function, childComponent);
	}

	template <class Func, class ChildComponentT>
	void IterateOnParents(Func function, const ChildComponentT& childComponent) const
	{
		static_assert(std::is_same_v<ChildComponentT, ChildComponent>);
		ChildComponentT::IterateOnParents(function, childComponent);
	}
};

}
