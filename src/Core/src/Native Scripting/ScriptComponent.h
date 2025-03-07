#pragma once

#include "DCoreAssert.h"
#include "AssetManagerTypes.h"
#include "ComponentRef.h"
#include "ReadWriteLockGuard.h"
#include "Registry.h"
#include "RootComponent.h"
#include "SerializationTypes.h"
#include "Component.h"
#include "EntityRef.h"
#include "PhysicsAPI.h"
#include "TransformComponent.h"
#include "NameComponent.h"
#include "UUIDComponent.h"
#include "BoxColliderComponent.h"
#include "AssetManager.h"

#include <utility>



namespace DCore
{

class Runtime;

class ScriptComponent : public Component
{
public:
	using rayCastResultType = Physics::RayCastResult;
	using overlapResultType = Physics::OverlapResult;
	using componentRefConstructorArgsType =	std::tuple<Entity, InternalSceneRefType, ComponentIdType, LockData*>;
public:
	virtual ~ScriptComponent() = default;
public:
	// Physics
	bool CastBox(float boxRotation, const DVec2& boxSizes, const DVec2& origin, const DVec2& direction, float maxDistance = FLT_MAX, uint64_t onlyCollideWithLayers = COLLIDE_WITH_ALL_MASK, rayCastResultType* out = nullptr);
	bool OverlapBox(float boxRotation, const DVec2& boxSizes, const DVec2& origin, uint64_t selfPhysicsLayer = UNDEFINED_PHYSICS_LAYER_MASK, uint64_t onlyCollideWithLayers = COLLIDE_WITH_ALL_MASK, overlapResultType* result = nullptr, size_t entitiesSize = 0);
	// Debug rendering
	void DrawDebugBox(const DVec2& translation, float rotation, const DVec2& sizes, const DVec4& color);
public:
	void Setup(EntityRef entityRef, ComponentIdType componentId)
	{
		m_entityRef = entityRef;
		m_componentId = componentId;
	}

	void SetRuntime(Runtime* runtime)
	{
		m_runtime = runtime;
	}
	
	ComponentIdType GetComponentId() const
	{
		return m_componentId;
	}

	componentRefConstructorArgsType GenerateComponentRefConstructorArgs()
	{
		return std::make_tuple(m_entityRef.GetEntity(), m_entityRef.GetInternalSceneRef(), m_componentId, m_entityRef.GetLockData());
	}
public:
	virtual void* GetAttributePtr(AttributeIdType) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(AttributeIdType, void* newValue, AttributeType typeHint) override
	{}

	virtual void Awake()
	{}

	virtual void Start() 
	{}

	virtual void Update(float deltaTime) 
	{}

	virtual void LateUpdate(float deltaTime) 
	{}

	virtual void PhysicsUpdate(float physicsDeltaTime) 
	{}

	virtual void PhysicsLateUpdate(float physicsDeltaTime) 
	{}

	virtual void AnimationUpdate(float animationDeltaTime)
	{}

	virtual void OnCollisionBegin(EntityRef other)
	{}

	virtual void OnCollisionEnd(EntityRef other)
	{}

	virtual void OnOverlapBegin(EntityRef other)
	{}

	virtual void OnOverlapEnd(EntityRef other)
	{}

	virtual void OnAnimationEvent(size_t eventId)
	{}
public:
	template <class ...Components, class ...TupleArgs>
	EntityRef CreateEntity(const char* entityName, TupleArgs&& ...tupleArgs)
	{
		DASSERT_E(m_entityRef.IsValid());
		Entity entity(m_entityRef.GetSceneRef().CreateEntity<RootComponent, TransformComponent, NameComponent, UUIDComponent, Components...>(
			std::make_tuple(), 
			std::make_tuple(ConstructorArgs<TransformComponent>()), 
			std::make_tuple(ConstructorArgs<NameComponent>(entityName)),
			std::make_tuple(ConstructorArgs<UUIDComponent>()),
			std::forward<TupleArgs>(tupleArgs)...));
		EntityRef entityRef(entity, m_entityRef.GetSceneRef());
		HandleComponentsInstantiation(entityRef, TypeList<Components...>());
		return entityRef;
	}

	template <class Func>
	void IterateOnEntitiesWithComponents(const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(m_entityRef.IsValid());
		m_entityRef.GetSceneRef().Iterate(componentIds, numberOfComponents, function);
	}

	template <class Func>
	void IterateOnEntitiesInAllScenesWithComponents(const ComponentIdType* componentIds, size_t numberOfComponents, Func function)
	{
		DASSERT_E(m_entityRef.IsValid());
		AssetManager::Get().IterateOnLoadedScenes(
			[&](SceneRef scene) -> bool
			{
				scene.Iterate(componentIds, numberOfComponents, function);
				return false;
			});
	}

protected:
	ScriptComponent() = default;
protected:
	EntityRef m_entityRef;
	ComponentIdType m_componentId;
	Runtime* m_runtime;
private:
	void HandleComponentsInstantiation(EntityRef entity, TypeList<>)
	{}
private:
	template <class Component, class ...Components>
	void HandleComponentsInstantiation(EntityRef entity, TypeList<Component, Components...>)
	{
		const ComponentIdType componentId(ComponentId::GetId<Component>());
		const ComponentForm& componentForm(ComponentForms::Get()[componentId]);
		if (componentForm.IsScriptComponent)
		{
			Registry& registry(entity.GetSceneRef().GetInternalSceneRef()->GetAsset().GetRegistry());
			registry.GetComponents
			(
				entity.GetEntity(), &componentId, 1,
				[&](ComponentIdType componentId, void* componentAddress) -> void
				{
					ScriptComponent* scriptComponent(static_cast<ScriptComponent*>(componentAddress));
					scriptComponent->Setup(entity, componentId);
					scriptComponent->SetRuntime(m_runtime);
					//scriptComponent->Awake();
					scriptComponent->Start();
				}
			);
		}
		if (componentId == ComponentId::GetId<BoxColliderComponent>())
		{
			SetupEntityPhysics(entity, *m_runtime);
		}
		HandleComponentsInstantiation(entity, TypeList<Components...>());
	}

	template <class RuntimeT>
	void SetupEntityPhysics(EntityRef entity, RuntimeT& runtime)
	{
		RuntimeT::SetupEntityPhysics(entity, runtime);
	}
};

template <>
class ComponentRef<ScriptComponent>
{
public:
	using constructorTupleType = std::tuple<Entity, InternalSceneRefType, ComponentIdType, LockData*>;
public:
	ComponentRef(
		Entity entity, 
		InternalSceneRefType internalSceneRef, 
		ComponentIdType componentId, 
		LockData& lockData)
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
		return m_lockData != nullptr && m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents(m_entity, &m_componentId, 1);
	}

	void Setup(EntityRef entity, ComponentIdType componentId)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->Setup(entity, componentId);
			}
		);
	}	

	void SetRuntime(Runtime* runtime)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->SetRuntime(runtime);
			}
		);
	}

	ComponentIdType GetComponentId() const
	{
		DASSERT_E(IsValid());
		return m_componentId;
	}

	void Awake()
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->Awake();
			}
		);
	}

	void Start()
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->Start();
			}
		);
	}

	void Update(float deltaTime)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->Update(deltaTime);
			}
		);
	}

	void LateUpdate(float deltaTime)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->LateUpdate(deltaTime);
			}
		);
	}

	void PhysicsUpdate(float physicsDeltaTime) 
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->PhysicsUpdate(physicsDeltaTime);
			}
		);
	}

	void PhysicsLateUpdate(float physicsDeltaTime)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->PhysicsLateUpdate(physicsDeltaTime);
			}
		);
	}

	void AnimationUpdate(float animationDeltaTime)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->AnimationUpdate(animationDeltaTime);
			}
		);
	}

	void OnCollisionBegin(EntityRef entity)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->OnCollisionBegin(entity);
			}
		);
	}

	void OnCollisionEnd(EntityRef entity)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->OnCollisionEnd(entity);
			}
		);
	}

	void OnOverlapBegin(EntityRef entity)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->OnOverlapBegin(entity);
			}
		);
	}

	void OnOverlapEnd(EntityRef entity)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->OnOverlapEnd(entity);
			}
		);
	}

	void OnAnimationEvent(size_t metachannelId)
	{
		DASSERT_E(IsValid());
		m_internalSceneRef->GetAsset().GetRegistry().GetComponents
		(
			m_entity, &m_componentId, 1,
			[&](ComponentIdType componentId, void* component) -> void
			{
				static_cast<ScriptComponent*>(component)->OnAnimationEvent(metachannelId);
			}
		);
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	ComponentIdType m_componentId;
	LockData* m_lockData;
};

}
