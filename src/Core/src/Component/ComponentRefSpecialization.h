#pragma once

#include "ComponentRef.h"
#include "Component.h"
#include "Asset.h"
#include "ECSTypes.h"
#include "Scene.h"
#include "ReadWriteLockGuard.h"
#include "DCoreAssert.h"

#include <atomic>
#include <cstring>



namespace DCore
{

template <>
class ComponentRef<Component>
{
public:
	using constructorTupleType = std::tuple<Entity, InternalSceneRefType, ComponentIdType, LockData*>;
public:
	ComponentRef()
		:
		m_lockData(nullptr)
	{}
	ComponentRef(Entity entity, InternalSceneRefType internalSceneRef, ComponentIdType componentId, LockData& lockData)
		:
		m_entity(entity),
		m_internalSceneRef(internalSceneRef),
		m_componentId(componentId),
		m_lockData(&lockData)
	{}
	ComponentRef(constructorTupleType args)
		:
		m_entity(std::get<0>(args)),
		m_internalSceneRef(std::get<1>(args)),
		m_componentId(std::get<2>(args)),
		m_lockData(std::get<3>(args))
	{}
	ComponentRef(const ComponentRef& other)
		:
		m_entity(other.m_entity),
		m_internalSceneRef(other.m_internalSceneRef),
		m_componentId(other.m_componentId),
		m_lockData(other.m_lockData)
	{}
	virtual ~ComponentRef() = default;
public:
	bool IsValid() const
	{
		return m_lockData != nullptr && m_entity.IsValid() && m_internalSceneRef.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents(m_entity, &m_componentId, 1);
	}

	void GetAttributePtr(AttributeIdType attributeId, void* out, size_t attributeSize)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* componentAddress) -> void
			{
				Component* component(static_cast<Component*>(componentAddress));
				std::memcpy(out, component->GetAttributePtr(attributeId), attributeSize);
			}
		);
	}

	void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<Component*>(component)->OnAttributeChange(attributeId, newValue, typeHint);
			}
		);
	}

	Component* GetRawComponent()
	{
		DASSERT_E(IsValid());
		Component* component(nullptr);
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* componentAddress) -> void
			{
				component = static_cast<Component*>(componentAddress);
			}
		);
		DASSERT_E(component != nullptr);
		return component;
	}
	
	ComponentIdType GetComponentId() const
	{
		DASSERT_E(IsValid());
		return m_componentId;
	}

	void Invalidate()
	{
		m_entity.Invalidate();
		m_internalSceneRef.Invalidate();
		m_componentId = 0;
		m_lockData = nullptr;
	}
public:
	ComponentRef& operator=(const ComponentRef& other)
	{
		m_entity = other.m_entity;
		m_internalSceneRef = other.m_internalSceneRef;
		m_componentId = other.m_componentId;
		m_lockData = other.m_lockData;
		return *this;
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	ComponentIdType m_componentId;
	LockData* m_lockData;
};

}
