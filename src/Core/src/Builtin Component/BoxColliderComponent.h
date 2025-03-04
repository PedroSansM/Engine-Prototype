#pragma once

#include "AssetManagerTypes.h"
#include "Component.h"
#include "ComponentForm.h"
#include "TemplateUtils.h"
#include "SerializationTypes.h"
#include "ComponentId.h"
#include "ComponentRef.h"
#include "ReadWriteLockGuard.h"
#include "DCoreAssert.h"
#include "Scene.h"
#include "Asset.h"
#include "PhysicsMaterial.h"
#include "PhysicsAPI.h"

#include "box2d/box2d.h"



namespace DCore
{

typedef 
struct BoxColliderComponentDirtyType
{
	static constexpr uint64_t BodyType					= static_cast<uint64_t>(1) << 0;
	static constexpr uint64_t Enabled					= static_cast<uint64_t>(1) << 1;
	static constexpr uint64_t Sensor					= static_cast<uint64_t>(1) << 2;
	static constexpr uint64_t GravityScale				= static_cast<uint64_t>(1) << 3;
	static constexpr uint64_t FixedRotation				= static_cast<uint64_t>(1) << 4;
	static constexpr uint64_t UseCCD					= static_cast<uint64_t>(1) << 5;
	static constexpr uint64_t Offset					= static_cast<uint64_t>(1) << 6;
	static constexpr uint64_t Size						= static_cast<uint64_t>(1) << 7;
	static constexpr uint64_t PhysicsMaterial			= static_cast<uint64_t>(1) << 8;
	static constexpr uint64_t SelfPhysicsLayer			= static_cast<uint64_t>(1) << 9;
	static constexpr uint64_t ColliderWithPhysicsLayer	= static_cast<uint64_t>(1) << 10;
	static constexpr uint64_t LinearVelocity			= static_cast<uint64_t>(1) << 11;

} BoxColliderComponentDirtyType;

using BoxColliderDirtyT = uint64_t;

class BoxColliderComponent;

#pragma pack(push, 1)
template <>
struct ConstructorArgs<BoxColliderComponent>
{
	ConstructorArgs()
		:
		BodyType(DBodyType::Static),
		Enabled(true),
		IsSensor(false),
		GravityScale(1.0),
		FixedRotation(false),
		UseCCD(false),
		Sizes{1.0f, 1.0f},
		SelfPhysicsLayer(DPhysicsLayer::Unspecified),
		CollideWithPhysicsLayers(DPhysicsLayer::Unspecified),
		DrawCollider(false)
	{}

	ConstructorArgs(
		DBodyType bodyType,
		DLogic enabled,
		DLogic isSensor,
		DFloat gravityScale,
		DLogic fixedRotation,
		DLogic useCCD,
		const DVec2& offset,
		const DVec2& sizes,
		PhysicsMaterialRef physicsMaterial,
		DPhysicsLayer selfPhysicsLayer,
		DPhysicsLayer collideWithPhysicsLayers)
		:
		BodyType(bodyType),
		Enabled(enabled),
		IsSensor(isSensor),
		GravityScale(gravityScale),
		FixedRotation(fixedRotation),
		UseCCD(useCCD),
		Offset(offset),
		Sizes(sizes),
		PhysicsMaterial(physicsMaterial),
		SelfPhysicsLayer(selfPhysicsLayer),
		CollideWithPhysicsLayers(collideWithPhysicsLayers),
		DrawCollider(false)
	{}
	
	DBodyType BodyType;
	DLogic Enabled;
	DLogic IsSensor;
	DFloat GravityScale;
	DLogic FixedRotation;
	DLogic UseCCD;
	DVec2 Offset;
	DVec2 Sizes;
	PhysicsMaterialRef PhysicsMaterial;
	DPhysicsLayer SelfPhysicsLayer;
	DPhysicsLayer CollideWithPhysicsLayers;
	DLogic DrawCollider;
};
#pragma pack(pop)

class BoxColliderComponent : public Component
{
public:
	static constexpr AttributeIdType a_bodyType{0};
	static constexpr AttributeIdType a_enabled{1};
	static constexpr AttributeIdType a_isSensor{2};
	static constexpr AttributeIdType a_gravityScale{3};
	static constexpr AttributeIdType a_fixedRotation{4};
	static constexpr AttributeIdType a_useCCD{5};
	static constexpr AttributeIdType a_offset{6};
	static constexpr AttributeIdType a_sizes{7};
	static constexpr AttributeIdType a_physicsMaterial{8};
	static constexpr AttributeIdType a_selfPhysicsLayer{9};
	static constexpr AttributeIdType a_collideWithPhysicsLayers{10};
	static constexpr AttributeIdType a_drawCollider{11};
public:
	BoxColliderComponent(const ConstructorArgs<BoxColliderComponent>&);
	~BoxColliderComponent();
public:
	virtual void* GetAttributePtr(AttributeIdType);
	virtual void OnAttributeChange(AttributeIdType, void* newValue, AttributeType typeHint);
public:
	DBodyType GetBodyType() const
	{
		return m_bodyType;
	}

	void SetBodyType(DBodyType value)
	{
		m_bodyType = value;
		m_dirtyType |= BoxColliderComponentDirtyType::BodyType;
	}

	DLogic IsEnabled() const
	{
		return m_enabled;
	}

	void SetEnabled(DLogic value)
	{
		m_enabled = value;
		m_dirtyType |= BoxColliderComponentDirtyType::Enabled;
	}

	DLogic IsSensor() const
	{
		return m_isSensor;
	}

	void SetIsSensor(DLogic value)
	{
		m_isSensor = value;
		m_dirtyType |= BoxColliderComponentDirtyType::Sensor;
	}

	DFloat GetGravityScale() const
	{
		return m_gravityScale;
	}

	void SetGravityScale(DFloat value)
	{
		m_gravityScale = value;
		m_dirtyType |= BoxColliderComponentDirtyType::GravityScale;
	}

	bool IsRotationFixed() const
	{
		return m_fixedRotation;
	}

	void SetFixedRotation(DLogic value)
	{
		m_fixedRotation = value;
		m_dirtyType |= BoxColliderComponentDirtyType::FixedRotation;
	}

	DLogic IsUsingCCD() const
	{
		return m_useCCD;
	}

	void SetUseCCD(DLogic value)
	{
		m_useCCD = value;
		m_dirtyType |= BoxColliderComponentDirtyType::UseCCD;
	}

	const DVec2& GetOffset() const
	{
		return m_offset;
	}

	void SetOffset(const DVec2& value)
	{
		m_offset = value;
		m_dirtyType |= BoxColliderComponentDirtyType::Offset;
	}

	const DVec2& GetSizes() const
	{
		return m_sizes;
	}

	void SetSizes(const DVec2& sizes)
	{
		m_sizes = {(std::max)(0.0f, sizes.x), (std::max)(0.0f, sizes.y)};
		m_dirtyType |= BoxColliderComponentDirtyType::Size;
	}

	PhysicsMaterialRef GetPhysicsMaterial() const
	{
		return m_physicsMaterial;
	}

	void SetPhysicsMaterial(PhysicsMaterialRef value)
	{
		m_physicsMaterial = value;
		m_dirtyType |= BoxColliderComponentDirtyType::PhysicsMaterial;
	}

	DPhysicsLayer GetSelfPhysicsLayer() const
	{
		return m_selfPhysicsLayer;
	}

	void SetSelfPhysicsLayer(DPhysicsLayer value)
	{
		m_selfPhysicsLayer = value;
		m_dirtyType |= BoxColliderComponentDirtyType::SelfPhysicsLayer;
	}

	DPhysicsLayer GetCollideWithPhysicsLayers() const
	{
		return m_collideWithPhysicsLayer;
	}

	void SetColliderWithPhysicsLayers(DPhysicsLayer value)
	{
		m_collideWithPhysicsLayer = value;
		m_dirtyType |= BoxColliderComponentDirtyType::ColliderWithPhysicsLayer;
	}

	DLogic HaveToDrawCollider() const
	{
		return m_drawCollider;
	}

	void SetDrawCollider(DLogic value)
	{
		m_drawCollider = value;
	}

	DBodyId GetBodyId() const
	{
		return m_bodyId;
	}

	void SetBodyId(b2BodyId value)
	{
		m_bodyId = value;
	}

	DShapeId GetShapeId() const
	{
		return m_shapeId;
	}

	void SetShapeId(DShapeId shapeId)
	{
		m_shapeId = shapeId;
	}

	void SetLinearVelocity(const DVec2& velocity)
	{
		m_linearVelocity = velocity;
		m_dirtyType |= BoxColliderComponentDirtyType::LinearVelocity;
	}

	const DVec2& GetLinearVelocity() const
	{
		return m_linearVelocity;
	}

	BoxColliderDirtyT GetDirtyType() const
	{
		return m_dirtyType;
	}

	void Clean()
	{
		m_dirtyType = 0;
	}
private:
	DBodyType m_bodyType;
	DLogic m_enabled;
	DLogic m_isSensor;
	DFloat m_gravityScale;
	DLogic m_fixedRotation;
	DLogic m_useCCD;
	DVec2 m_offset;
	DVec2 m_sizes;
	PhysicsMaterialRef m_physicsMaterial;
	DPhysicsLayer m_selfPhysicsLayer;
	DPhysicsLayer m_collideWithPhysicsLayer;
	DLogic m_drawCollider;
	DBodyId m_bodyId;
	DShapeId m_shapeId;
	DVec2 m_linearVelocity;
 	BoxColliderDirtyT m_dirtyType;
};

class BoxColliderComponentFormGenerator : public ComponentFormGenerator
{
public:
	~BoxColliderComponentFormGenerator() = default;
private:
	BoxColliderComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<BoxColliderComponent>(),
				"Box Collider Component",
				false,
				sizeof(BoxColliderComponent),
				sizeof(ConstructorArgs<BoxColliderComponent>),
				{{AttributeName("Type"), AttributeType::PhysicsBodyType, BoxColliderComponent::a_bodyType},
				{AttributeName("Enabled"), AttributeType::Logic, BoxColliderComponent::a_enabled},
				{AttributeName("Is Sensor"), AttributeType::Logic, BoxColliderComponent::a_isSensor},
				{AttributeName("Gravity Scale"), AttributeType::Float, BoxColliderComponent::a_gravityScale},
				{AttributeName("Fixed Rotation"), AttributeType::Logic, BoxColliderComponent::a_fixedRotation},
				{AttributeName("Use CCD"), AttributeType::Logic, BoxColliderComponent::a_useCCD},
				{AttributeName("Offset#X#Y"), AttributeType::Vector2, BoxColliderComponent::a_offset},
				{AttributeName("Sizes#X#Y"), AttributeType::Vector2, BoxColliderComponent::a_sizes},
				{AttributeName("Physics Material"), AttributeType::PhysicsMaterial, BoxColliderComponent::a_physicsMaterial},
				{AttributeName("Self Physics Layer"), AttributeType::PhysicsLayer, BoxColliderComponent::a_selfPhysicsLayer},
				{AttributeName("Collide With Physics Layer"), AttributeType::PhysicsLayers, BoxColliderComponent::a_collideWithPhysicsLayers},
				{AttributeName("Draw Debug Collider"), AttributeType::Logic, BoxColliderComponent::a_drawCollider}},
				[](void* address, const void* args) -> void
				{
					new (address) BoxColliderComponent(*static_cast<const ConstructorArgs<BoxColliderComponent>*>(args));
				},
				[](void* componentAddess) -> void
				{
					static_cast<BoxColliderComponent*>(componentAddess)->~BoxColliderComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	ConstructorArgs<BoxColliderComponent> m_defaultArgs;
private:
	static BoxColliderComponentFormGenerator s_generator;
};

template <>
class ComponentRef<BoxColliderComponent>
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
	bool IsValid() const
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<BoxColliderComponent>(m_entity);
	}

	void GetAttributePtr(AttributeIdType attributeId, void* out, size_t attributeSize)
	{
		DASSERT_E(IsValid());
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		std::memcpy(out, boxColliderComponent.GetAttributePtr(attributeId), attributeSize);
	}

	void OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType typeHint)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.OnAttributeChange(attributeId, newValue, typeHint);
	}
	
	DBodyType GetBodyType() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetBodyType();
	}

	void SetBodyType(DBodyType value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetBodyType(value);
	}

	DLogic IsEnabled() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.IsEnabled();
	}
	
	void SetEnabled(DLogic value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetEnabled(value);
	}

	DLogic IsSensor() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.IsSensor();
	}
 
	void SetIsSensor(DLogic value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetIsSensor(value);
	}

	DFloat GetGravityScale() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetGravityScale();
	}

	void SetGravityScale(DLogic value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetGravityScale(value);
	}

	DLogic IsRotationFixed() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.IsRotationFixed();
	}

	void SetFixedRotation(DLogic value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetFixedRotation(value);
	}

	DLogic IsUsingCCD() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.IsUsingCCD();
	}

	void SetUseCCD(DLogic value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetUseCCD(value);
	}

	DVec2 GetOffset() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetOffset();
	}

	void SetOffset(const DVec2& value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetOffset(value);
	}

	DVec2 GetSizes() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetSizes();
	}

	void SetSizes(const DVec2& value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetSizes(value);
	}

	PhysicsMaterialRef GetPhysicsMaterial() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetPhysicsMaterial();
	}

	void SetPhysicsMaterial(PhysicsMaterialRef value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetPhysicsMaterial(value);
	}

	DPhysicsLayer GetSelfPhysicsLayer() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetSelfPhysicsLayer();
	}

	void SetSelfPhysicsLayer(DPhysicsLayer value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetSelfPhysicsLayer(value);
	}

	DPhysicsLayer GetCollideWithPhysicsLayers() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetCollideWithPhysicsLayers();
	}

	void SetColliderWithPhysicsLayers(DPhysicsLayer value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetColliderWithPhysicsLayers(value);
	}

	DLogic HaveToDrawCollider() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.HaveToDrawCollider();
	}

	void SetDrawCollider(DLogic value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetDrawCollider(value);
	}

	DBodyId GetBodyId() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetBodyId();
	}

	void SetBodyId(DBodyId value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetBodyId(value);
	}

	DShapeId GetShapeId() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetShapeId();
	}

	void SetShapeId(DShapeId value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetShapeId(value);
	}

	BoxColliderDirtyT GetDirtyType() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetDirtyType();	
	}

	DVec2 GetLinearVelocity() const
	{
		DASSERT_E(IsValid());
		const BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		return boxColliderComponent.GetLinearVelocity();	
	}

	void SetLinearVelocity(const DVec2& velocity)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.SetLinearVelocity(velocity);
	}

	void Clean()
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		BoxColliderComponent& boxColliderComponent(m_internalSceneRef->GetAsset().GetRegistry().GetComponents<BoxColliderComponent>(m_entity));
		boxColliderComponent.Clean();	
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
