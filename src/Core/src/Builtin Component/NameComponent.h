#pragma once

#include "DCoreAssert.h"
#include "AssetManagerTypes.h"
#include "ComponentId.h"
#include "TemplateUtils.h"
#include "ComponentForm.h"
#include "Component.h"
#include "SerializationTypes.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"
#include "ReadWriteLockGuard.h"

#include <cstring>
#include <atomic>



namespace DCore
{

class NameComponent;
 
template <>
struct ConstructorArgs<NameComponent>
{
	ConstructorArgs()
		:
		Name("Name")
	{}

	ConstructorArgs(const char* name)
		:
		Name(name)
	{}

	DString Name;
};

class NameComponent : public Component
{
public:
	NameComponent(const ConstructorArgs<NameComponent>& args);
	~NameComponent() = default;
public:
	void* GetAttributePtr(AttributeIdType attributeId) override
	{
		switch (attributeId) 
		{
		case 0:
			return &m_name;
		default:
			return nullptr;
		}
	}

	void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint) override
	{
		switch (attributeId) 
		{
		case 0:
			m_name = *static_cast<const DString*>(newValue);
			break;
		default:
			DASSERT_E(false);
			break;
		}
	}
public:
	const DString& GetName() const
	{
		return m_name;
	}

	void SetName(const DString& name)
	{
		m_name = name;
	}
private:
	DString m_name;
};

class NameComponentFormGenerator : private ComponentFormGenerator
{
public:
	~NameComponentFormGenerator() = default;
private:
	NameComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<NameComponent>(),
				"Name Component",
				false,
				sizeof(NameComponent),
				sizeof(ConstructorArgs<NameComponent>),
				{{AttributeName("Name"), AttributeType::String, 0}},
				[](void* componentAddress, const void* args) -> void
				{
					new (componentAddress) NameComponent(*static_cast<const ConstructorArgs<NameComponent>*>(args));
				},
				[](void* componentAddress) -> void
				{
					static_cast<NameComponent*>(componentAddress)->~NameComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	ConstructorArgs<NameComponent> m_defaultArgs;
private:
	static NameComponentFormGenerator s_generator;
};

template <>
class ComponentRef<NameComponent>
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
		DASSERT_E(IsValid());
		NameComponent& nameComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<NameComponent>(m_entity));
		std::memcpy(out, nameComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		NameComponent& nameComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<NameComponent>(m_entity));
		nameComponent.OnAttributeChange(attributeId, newValue, typeHint);
	}	
public:
	bool IsValid() const
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<NameComponent>(m_entity);
	}

	void GetName(DString& outName) const
	{
		DASSERT_E(IsValid());
		const NameComponent& nameComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<NameComponent>(m_entity));
		outName = nameComponent.GetName().Data();
	}

	void SetName(const DString& name)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		NameComponent& nameComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<NameComponent>(m_entity));
		nameComponent.SetName(name);
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
