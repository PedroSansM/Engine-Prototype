#pragma once

#include "DCoreAssert.h"
#include "AssetManagerTypes.h"
#include "ComponentId.h"
#include "ReadWriteLockGuard.h"
#include "SerializationTypes.h"
#include "TemplateUtils.h"
#include "Component.h"
#include "ChildComponent.h"
#include "AssetManager.h"
#include "EntityRef.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"

#include <cassert>
#include <cstring>
#include <functional>
#include <vector>
#include <atomic>



namespace DCore
{

class ChildrenComponent;

#pragma pack(push, 1)
template <>
struct ConstructorArgs<ChildrenComponent>
{
	ConstructorArgs()
		:
		NumberOfChildren(0)
	{}

	EntityRef FirstChild;
	DSize NumberOfChildren;
};
#pragma pack(pop)

class ChildrenComponent : public Component
{
public:
	ChildrenComponent(const ConstructorArgs<ChildrenComponent>& args);
	~ChildrenComponent() = default;
public:
	template <class Func>
	static void IterateOnChildren(Func function, ChildrenComponent& childrenComponent)
	{
		childrenComponent.IterateOnChildren(function);
	}
public:
	// SceneAssetManager must be locked for writing.
	void AddChild(EntityRef);
	
	// SceneAssetManager must be locked for writing.
	void RemoveChild(EntityRef);
	
	bool HaveChild(const EntityRef) const;
	std::vector<EntityRef> GetChildren() const;
public:
	void* GetAttributePtr(AttributeIdType attributeId)
	{
		switch (attributeId) 
		{
		case 0:
			return &m_firstChild;
		case 1:
			return &m_numberOfChildren;
		default:
			return nullptr;
		}
	}
public:
	size_t GetNumberOfChildren() const
	{
		return m_numberOfChildren;
	}

	EntityRef GetFirstChild() const
	{
		return m_firstChild;
	}
public:
	template <class Func>
	void IterateOnChildren(Func function) const
	{
		EntityRef currentEntity(m_firstChild);
		for (size_t i(0); i < m_numberOfChildren; i++)
		{
			if (!currentEntity.IsValid())
			{
				continue;
			}
			bool toStop(std::invoke(function, currentEntity));
			if (toStop)
			{
				return;
			}
			ComponentRef<ChildComponent> currentChildComponent;
			if (!currentEntity.TryGetComponents<ChildComponent>(currentChildComponent))
			{
				return;
			}
			currentEntity = currentChildComponent.GetNext();
		}
	}
private:
	EntityRef m_firstChild;
	DSize m_numberOfChildren;
};

class ChildrenComponentFormGenerator : public ComponentFormGenerator
{
public:
	~ChildrenComponentFormGenerator() = default;
private:
	ChildrenComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<ChildrenComponent>(),
				"Children Component",
				false,
				sizeof(ChildrenComponent),
				sizeof(ConstructorArgs<ChildrenComponent>),
				{{AttributeName("First Child"), AttributeType::EntityReference, 0}, 
				{AttributeName("Number Of Children"), AttributeType::Size, 1}},
				[](void* address, const void* args) -> void
				{
					new (address) ChildrenComponent(*static_cast<const ConstructorArgs<ChildrenComponent>*>(args));
				},
				[](void* componentAddress) -> void
				{
					static_cast<ChildrenComponent*>(componentAddress)->~ChildrenComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	ConstructorArgs<ChildrenComponent> m_defaultArgs;
private:
	static ChildrenComponentFormGenerator s_generator;
};

template <>
class ComponentRef<ChildrenComponent> 
{
public:
	ComponentRef(Entity entity, InternalSceneRefType internalSceneRef, LockData& lockData)
		:
		m_entity(entity),
		m_internalSceneRef(internalSceneRef),
		m_lockData(&lockData)
	{}
	~ComponentRef() = default;
public:
	void GetAttrbutePtr(AttributeIdType attributeId, void* out, size_t attributeSize)
	{
		if (m_lockData == nullptr)
		{
			return;
		}
		if (!IsValid())
		{
			return;
		}
		ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		std::memcpy(out, childrenComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId)
	{}
public:
	bool IsValid() const
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<ChildrenComponent>(m_entity);
	}
	
	void AddChild(EntityRef child)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		childrenComponent.AddChild(child);
	}

	void RemoveChild(EntityRef child)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		childrenComponent.RemoveChild(child);
	}

	bool HaveChild(EntityRef child) const
	{
		DASSERT_E(IsValid());
		const ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		return childrenComponent.HaveChild(child);
	}

	std::vector<EntityRef> GetChildren() const
	{
		DASSERT_E(IsValid());
		const ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		return childrenComponent.GetChildren();
	}

	size_t GetNumberOfChildren() const
	{
		DASSERT_E(IsValid());
		const ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		return childrenComponent.GetNumberOfChildren();
	}

	EntityRef GetFirstChild() const
	{
		DASSERT_E(IsValid());
		const ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		return childrenComponent.GetFirstChild();
	}
public:
	template <class Func>
	void IterateOnChildren(Func function)
	{
		DASSERT_E(IsValid());
		ChildrenComponent& childrenComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildrenComponent>(m_entity));
		childrenComponent.IterateOnChildren(function);
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
