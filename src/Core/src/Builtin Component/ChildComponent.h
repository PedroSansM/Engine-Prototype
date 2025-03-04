#pragma once

#include "DCoreAssert.h"
#include "ComponentId.h"
#include "ReadWriteLockGuard.h"
#include "SerializationTypes.h"
#include "TemplateUtils.h"
#include "Component.h"
#include "AssetManager.h"
#include "AssetManagerTypes.h"
#include "UUID.h"
#include "EntityRef.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"
#include "AttributeName.h"

#include <atomic>
#include <cstring>



namespace DCore
{

class ChildComponent;

template <>
struct ConstructorArgs<ChildComponent>
{
	ConstructorArgs()
	{}

	ConstructorArgs(EntityRef parent)
		:
		Parent(parent)
	{}

	EntityRef Parent;
	EntityRef Next;
	EntityRef Previous;
};

class ChildComponent : public Component
{
public:
	static constexpr size_t a_parent{0};
	static constexpr size_t a_next{1};
	static constexpr size_t a_previous{2};
public:
	ChildComponent(const ConstructorArgs<ChildComponent>& args);
	~ChildComponent() = default;
public:
	template <class Func>
	static void IterateOnParents(Func function, ChildComponent& childComponent)
	{
		childComponent.IterateOnParents(function);
	}

	template <class Func>
	static void IterateOnParents(Func function, const ChildComponent& childComponent)
	{
		childComponent.IterateOnParents(function);
	}
public:
	virtual void* GetAttributePtr(AttributeIdType attributeId) override
	{
		switch (attributeId) 
		{
		case a_parent:
			return &m_parent;
		case a_next:
			return &m_next;
		case a_previous:
			return &m_previous;
		default:
			return nullptr;
		}
	}
public:
	void SetParent(EntityRef parent)
	{
		m_parent = parent;
	}	

	EntityRef GetParent() const
	{
		return m_parent;
	}

	void SetNext(EntityRef next)
	{
		m_next = next;
	}
	
	void SetPrevious(EntityRef previous)
	{
		m_previous = previous;
	}

	EntityRef GetNext() const
	{
		return m_next;
	}

	EntityRef GetPrevious() const
	{
		return m_previous;
	}

	bool HaveNext() const
	{
		return m_next.IsValid();
	}

	bool HavePrevious() const
	{
		return m_previous.IsValid();
	}

	void RemoveNext()
	{
		m_next.Invalidate();
	}

	void RemovePrevious()
	{
		m_previous.Invalidate();
	}
public:
	template <class Func>
	void IterateOnParents(Func function)
	{
		EntityRef currentEntity(m_parent);
		InternalSceneRefType scene(currentEntity.GetInternalSceneRef());
		while (true)
		{
			if (std::invoke(function, currentEntity))
			{
				return;
			}
			if (!currentEntity.HaveComponents<ChildComponent>())
			{
				return;
			}
			ChildComponent& childComponent(scene->GetAsset().GetRegistry().GetComponents<ChildComponent>(currentEntity.GetEntity()));
			currentEntity = childComponent.GetParent();
		} 
	}

	template <class Func>
	void IterateOnParents(Func function) const
	{
		EntityRef currentEntity(m_parent);
		InternalSceneRefType scene(currentEntity.GetInternalSceneRef());
		while (true)
		{
			if (std::invoke(function, currentEntity))
			{
				return;
			}
			if (!currentEntity.HaveComponents<ChildComponent>())
			{
				return;
			}
			const ChildComponent& childComponent(scene->GetAsset().GetRegistry().GetComponents<ChildComponent>(currentEntity.GetEntity()));
			currentEntity = childComponent.GetParent();
		} 
	}
private:
	EntityRef m_parent;
	EntityRef m_next;
	EntityRef m_previous;
};

class ChildComponentFormGenerator : private ComponentFormGenerator
{
public:
	~ChildComponentFormGenerator() = default;
private:
	ChildComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<ChildComponent>(),
				"Child Component",
				false,
				sizeof(ChildComponent),
				sizeof(ConstructorArgs<ChildComponent>),
				{{AttributeName("Parent"), AttributeType::EntityReference, ChildComponent::a_parent}, 
				{AttributeName("Next"), AttributeType::EntityReference, ChildComponent::a_next}, 
				{AttributeName("Previous"), AttributeType::EntityReference, ChildComponent::a_previous}}, 
				[](void* address, const void* args) -> void
				{
					new (address) ChildComponent(*static_cast<const ConstructorArgs<ChildComponent>*>(args));
				},
				[](void* address) -> void
				{
					 static_cast<ChildComponent*>(address)->~ChildComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	ConstructorArgs<ChildComponent> m_defaultArgs;
private:
	static ChildComponentFormGenerator s_generator;
};

template <>
class ComponentRef<ChildComponent> 
{
public:
	ComponentRef() = default;
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
		DASSERT_E(IsValid());
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		std::memcpy(out, childComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId)
	{}

	ChildComponent& GetRawComponent()
	{
		DASSERT_E(IsValid());
		return m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity);
	}
public:
	bool IsValid() const
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<ChildComponent>(m_entity);
	}

	void SetParent(EntityRef parent)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		childComponent.SetParent(parent);
	}

	EntityRef TryGetParent() const
	{
		DASSERT_E(IsValid());
		const ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		return childComponent.GetParent();
	}

	void SetNext(EntityRef next)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		childComponent.SetNext(next);
	}

	void SetPrevious(EntityRef previous)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		childComponent.SetPrevious(previous);
	}

	EntityRef GetNext() const
	{
		DASSERT_E(IsValid());
		const ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		if (!childComponent.HaveNext())
		{
			return EntityRef();
		}
		return childComponent.GetNext();
	}

	EntityRef GetPrevious() const
	{
		DASSERT_E(IsValid());
		const ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		return childComponent.GetPrevious();
	}

	bool HaveNext()
	{
		DASSERT_E(IsValid());
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		return childComponent.HaveNext();
	}

	bool HavePrevious()
	{
		DASSERT_E(IsValid());
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		return childComponent.HavePrevious();
	}

	void RemoveNext()
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		childComponent.RemoveNext();
	}

	void RemovePrevious()
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		childComponent.RemovePrevious();
	}
public:
	template <class Func>
	void IterateOnParents(Func function)
	{
		DASSERT_E(IsValid());
		ChildComponent& childComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<ChildComponent>(m_entity));
		childComponent.IterateOnParents(function);
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
