#pragma once

#include "AssetManagerTypes.h"
#include "Component.h"
#include "ComponentForm.h"
#include "ComponentId.h"
#include "TemplateUtils.h"
#include "UUID.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"
#include "DCoreAssert.h"
#include "ReadWriteLockGuard.h"

#include <atomic>
#include <cstring>



namespace DCore
{

class UUIDComponent;

template <>
struct ConstructorArgs<UUIDComponent>
{
	UUIDType UUID;
};

class UUIDComponent : public Component
{
public:
	UUIDComponent(const ConstructorArgs<UUIDComponent>&);
	~UUIDComponent() = default;
public:
	const UUIDType& GetUUID() const
	{
		return m_uuid;
	}

	virtual void* GetAttributePtr(AttributeIdType attributeId) override
	{
		switch (attributeId) 
		{
		case 0:
			return &m_uuid;
		default:
			return nullptr;
		}
	}

	virtual void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType attributeType) override
	{
		switch (attributeId)
		{
		case 0:
			m_uuid = *static_cast<UUIDType*>(newValue);
			return;
		default:
			return;
		}
	}
private:
	UUIDType m_uuid;
};

class UUIDComponentFormGenerator : private ComponentFormGenerator
{
public:
	~UUIDComponentFormGenerator() = default;
private:
	UUIDComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<UUIDComponent>(),
				"UUID Component",
				false,
				sizeof(UUIDComponent),
				sizeof(ConstructorArgs<UUIDComponent>),
				{SerializedAttribute(AttributeName("UUID"), AttributeType::UUID, 0)},
				[](void* address, const void* args) -> void
				{
					new (address) UUIDComponent(*(const ConstructorArgs<UUIDComponent>*)args);
				},
				[](void* componentAddress) -> void
				{
					static_cast<UUIDComponent*>(componentAddress)->~UUIDComponent();
				},
				&m_defaultArgs
			}
		)
{}
private:
	ConstructorArgs<UUIDComponent> m_defaultArgs;
private:
	static UUIDComponentFormGenerator s_generator;
};

template <>
class ComponentRef<UUIDComponent> 
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
		UUIDComponent& uuidComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<UUIDComponent>(m_entity));
		std::memcpy(out, uuidComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId)
	{}
public:
	bool IsValid()
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<UUIDComponent>(m_entity);
	}	

	void GetUUID(UUIDType& outUUID)
	{
		DASSERT_E(IsValid());
		UUIDComponent& uuidComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<UUIDComponent>(m_entity));
		outUUID = uuidComponent.GetUUID();
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
