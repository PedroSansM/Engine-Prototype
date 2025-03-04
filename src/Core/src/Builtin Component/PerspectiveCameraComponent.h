#pragma once

#include "DCoreAssert.h"
#include "AssetManagerTypes.h"
#include "ComponentId.h"
#include "SerializationTypes.h"
#include "TemplateUtils.h"
#include "Component.h"
#include "ComponentForm.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"
#include "ReadWriteLockGuard.h"

#include <atomic>
#include <cstring>



namespace DCore
{

class PerspectiveCameraComponent;

template <>
struct ConstructorArgs<PerspectiveCameraComponent>
{
	ConstructorArgs()
		:
		FOV(60),
		Near(0.3f),
		Far(1000.0f)
	{}

	DUInt FOV;
	DFloat Near;
	DFloat Far;
};

class PerspectiveCameraComponent : public Component
{
public:
	static constexpr AttributeIdType a_fov{0};
	static constexpr AttributeIdType a_near{1};
	static constexpr AttributeIdType a_far{2};
public:
	PerspectiveCameraComponent(ConstructorArgs<PerspectiveCameraComponent>&);
	PerspectiveCameraComponent(ConstructorArgs<PerspectiveCameraComponent>&&);
	~PerspectiveCameraComponent() = default;
public:
	void OnAttributeChange(AttributeIdType, void* newValue, AttributeType typeHint) override;
public:
	DMat4 GetProjectionMatrix(const DVec2& viewportSizes) const;
public:
	void* GetAttributePtr(AttributeIdType attributeId) override
	{
		switch (attributeId) 
		{
		case a_fov:
			return &m_fov;
		case a_near:
			return &m_near;
		case a_far:
			return &m_far;
		default:
			return nullptr;
		}
	}
public:
	DUInt GetFOV() const
	{
		return m_fov;
	}

	DFloat GetNear() const
	{
		return m_near;
	}

	inline DFloat GetFar() const
	{
		return m_far;
	}
private:
	DUInt m_fov;
	DFloat m_near;
	DFloat m_far;
};

class PerspectiveCameraComponentFormGenerator : public ComponentFormGenerator
{
public:
	~PerspectiveCameraComponentFormGenerator() = default;
private:
	PerspectiveCameraComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<PerspectiveCameraComponent>(),
				"Perspective Camera",
				false,
				sizeof(PerspectiveCameraComponent),
				sizeof(ConstructorArgs<PerspectiveCameraComponent>),
				{{AttributeName("Field of View"), AttributeType::UInteger, PerspectiveCameraComponent::a_fov}, 
				{AttributeName("Near"), AttributeType::Float, PerspectiveCameraComponent::a_near}, 
				{AttributeName("Far"), AttributeType::Float, PerspectiveCameraComponent::a_far}},
				[](void* componentAddress, const void* args) -> void
				{
					new (componentAddress) PerspectiveCameraComponent(*(ConstructorArgs<PerspectiveCameraComponent>*)args);
				},
				[](void* componentAddress) -> void
				{
					static_cast<PerspectiveCameraComponent*>(componentAddress)->~PerspectiveCameraComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	ConstructorArgs<PerspectiveCameraComponent> m_defaultArgs;
private:
	static PerspectiveCameraComponentFormGenerator s_generator;
};

template <>
class ComponentRef<PerspectiveCameraComponent> 
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
		PerspectiveCameraComponent& perspectiveCameraComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<PerspectiveCameraComponent>(m_entity));
		std::memcpy(out, perspectiveCameraComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		PerspectiveCameraComponent& perspectiveCameraComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<PerspectiveCameraComponent>(m_entity));
		perspectiveCameraComponent.OnAttributeChange(attributeId, newValue, typeHint);
	}
public:
	bool IsValid()
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<PerspectiveCameraComponent>(m_entity);
	}

	DMat4 GetProjectionMatrix(const DVec2& viewportSizes)
	{
		DASSERT_E(IsValid());
		PerspectiveCameraComponent& perspectiveCameraComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<PerspectiveCameraComponent>(m_entity));
		return perspectiveCameraComponent.GetProjectionMatrix(viewportSizes);
	}

	DUInt GetFOV()
	{
		DASSERT_E(IsValid());
		PerspectiveCameraComponent& perspectiveCameraComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<PerspectiveCameraComponent>(m_entity));
		return perspectiveCameraComponent.GetFOV();
	}

	DFloat GetNear()
	{
		DASSERT_E(IsValid());
		PerspectiveCameraComponent& perspectiveCameraComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<PerspectiveCameraComponent>(m_entity));
		return perspectiveCameraComponent.GetNear();
	}

	DFloat GetFar()
	{
		DASSERT_E(IsValid());
		PerspectiveCameraComponent& perspectiveCameraComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<PerspectiveCameraComponent>(m_entity));
		return perspectiveCameraComponent.GetFar();
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
