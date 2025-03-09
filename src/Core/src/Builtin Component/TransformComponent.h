#pragma once

#include "DCoreAssert.h"
#include "AssetManagerTypes.h"
#include "Component.h"
#include "ComponentForm.h"
#include "ComponentId.h"
#include "TemplateUtils.h"
#include "SerializationTypes.h"
#include "ComponentRef.h"
#include "Asset.h"
#include "Scene.h"
#include "ReadWriteLockGuard.h"
#include "AttributeName.h"

#include <atomic>



namespace DCore
{

class TransformComponent;

template <>
struct ConstructorArgs<TransformComponent>
{
	ConstructorArgs()
		:
		Translation(0.0f, 0.0f, 0.0f),
		Rotation(0.0f),
		Scale(1.0f, 1.0f)
	{}

	ConstructorArgs(DVec3 translation, DFloat rotation, DVec2 scale)
		:
		Translation(translation),
		Rotation(rotation),
		Scale(scale)
	{}

	DVec3 Translation;
	DFloat Rotation;
	DVec2 Scale;
};

class TransformComponent : public Component
{
public:
	static constexpr AttributeIdType a_translation{0};
	static constexpr AttributeIdType a_rotation{1};
	static constexpr AttributeIdType a_scale{2};
public:
	TransformComponent(const ConstructorArgs<TransformComponent>&);
	TransformComponent(ConstructorArgs<TransformComponent>&&);
	~TransformComponent() = default;
public:
	virtual void OnAttributeChange(AttributeIdType, void* newValue, AttributeType typeHint) override;
	DMat4 GetInverseModelMatrix() const;
public:
	virtual void* GetAttributePtr(AttributeIdType attributeId) override
	{
		switch (attributeId) 
		{
		case a_translation:
			return &m_translation;
		case a_rotation:
			return &m_rotation;
		case a_scale:
			return &m_scale;
		default:
			return nullptr;
		}
	}
public:
	DVec3 GetTranslation() const
	{
		return m_translation;
	}

	DVec3& GetTranslation()
	{
		return m_translation;
	}

	DFloat GetRotation() const
	{
		return m_rotation;
	}

	DFloat& GetRotation()
	{
		return m_rotation;
	}

	DVec2 GetScale() const
	{
		return m_scale;
	}

	DVec2& GetScale()
	{
		return m_scale;
	}

	void SetTranslation(const DVec3 translation)
	{
		m_translation = translation;
		UpdateModelMatrix();
		m_isDirty = true;
	}

	void AddTranslation(const DVec3& translation)
	{
		m_translation += translation;
		UpdateModelMatrix();
		m_isDirty = true;
	}
	 
	void SetRotation(DFloat rotation) 
	{
		while (rotation >= 360.0f)
		{
			rotation -= 360.0f;
		}
		while (rotation <= -360.0f)
		{
			rotation += 360.0f;
		}
		if (rotation > 90.0f)
		{
			rotation -= 180.0f;
		}
		else if (rotation < -90.0f)
		{
			rotation += 180.0f;
		}
		m_rotation = rotation;
		UpdateModelMatrix();
		m_isDirty = true;
	}

	void SetScale(DVec2 scale)
	{
		m_scale = scale;
		UpdateModelMatrix();
		m_isDirty = true;
	}

	const DMat4& GetModelMatrix() const
	{
		return m_modelMatrix;
	}

	bool IsDirty() const
	{
		return m_isDirty;
	}

	void SetIsDirty(bool value)
	{
		m_isDirty = value;
	}
private:
	DVec3 m_translation;
	DFloat m_rotation;
	DVec2 m_scale;
	DMat4 m_modelMatrix;
	bool m_isDirty;
private:
	void UpdateModelMatrix();
};

class TransformComponentFormGenerator : private ComponentFormGenerator
{
public:
	~TransformComponentFormGenerator() = default;
private:
	TransformComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<TransformComponent>(),
				"Transform Component",
				false,
				sizeof(TransformComponent),
				sizeof(ConstructorArgs<TransformComponent>),
				{{AttributeName("Translation#X#Y#Z"), AttributeType::Vector3, TransformComponent::a_translation}, 
				{AttributeName("Rotation"), AttributeType::Float, TransformComponent::a_rotation}, 
				{AttributeName("Scale#X#Y"), AttributeType::Vector2, TransformComponent::a_scale}},
				[](void* address, const void* args) -> void
				{
					new (address) TransformComponent(*(const ConstructorArgs<TransformComponent>*)args);
				},
				[](void* componentAddress) -> void
				{
					static_cast<TransformComponent*>(componentAddress)->~TransformComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	ConstructorArgs<TransformComponent> m_defaultArgs;
private:
	static TransformComponentFormGenerator s_generator;
};

template <>
class ComponentRef<TransformComponent>
{
public:
	ComponentRef()
		:
		m_lockData(nullptr)
	{}
	ComponentRef(Entity entity, InternalSceneRefType internalSceneRef, LockData& lockData)
		:
		m_entity(entity),
		m_internalSceneRef(internalSceneRef),
		m_lockData(&lockData)
	{}
	~ComponentRef() = default;
public:
	void GetAttributePtr(AttributeIdType attributeId, void* out, size_t attributeSize)
	{
		DASSERT_E(IsValid());
		TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		std::memcpy(out, transformComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
	{
		DASSERT_E(IsValid());
		TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		transformComponent.OnAttributeChange(attributeId, newValue, typeHint);
	}
public:
	bool IsValid() const
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<TransformComponent>(m_entity);
	}

	DMat4 GetInverseModelMatrix() const
	{
		DASSERT_E(IsValid());
		const TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		return transformComponent.GetInverseModelMatrix();		
	}

	DVec3 GetTranslation() const
	{
		DASSERT_E(IsValid());
		const TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		return transformComponent.GetTranslation();
	}

	DFloat GetRotation() const
	{
		DASSERT_E(IsValid());
		const TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		return transformComponent.GetRotation();
	}

	DVec2 GetScale() const
	{
		DASSERT_E(IsValid());
		const TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		return transformComponent.GetScale();
	}

	void SetTranslation(const DVec3& translation)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		transformComponent.SetTranslation(translation);
	}

	void AddTranslation(const DVec3& translation)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		transformComponent.AddTranslation(translation);
	}

	void SetRotation(DFloat rotation)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		transformComponent.SetRotation(rotation);
	}

	void SetScale(DVec2 scale)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		transformComponent.SetScale(scale);
	}

	DMat4 GetModelMatrix() const
	{
		DASSERT_E(IsValid());
		const TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		return transformComponent.GetModelMatrix();
	}

	bool IsDirty() const
	{
		DASSERT_E(IsValid());
		const TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		return transformComponent.IsDirty();
	}

	void SetIsDirty(bool value)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		TransformComponent& transformComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<TransformComponent>(m_entity));
		transformComponent.SetIsDirty(value);
	}
public:
	ComponentRef& operator=(const ComponentRef other)
	{
		m_entity = other.m_entity;
		m_internalSceneRef = other.m_internalSceneRef;
		m_lockData = other.m_lockData;
		return *this;
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
