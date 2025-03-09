#pragma once

#include "AttributeName.h"
#include "Component.h"
#include "ComponentId.h"
#include "TemplateUtils.h"
#include "AnimationStateMachine.h"
#include "SerializationTypes.h"
#include "ComponentForm.h"
#include "ReadWriteLockGuard.h"

#include <string>



namespace DCore
{

class AnimationStateMachineComponent;

template <>
struct ConstructorArgs<AnimationStateMachineComponent>
{
	DAnimationStateMachine AnimationStateMachine;
};

class AnimationStateMachineComponent : public Component
{
public:
	using stringType = std::string;
public:
	static constexpr AttributeIdType a_animationStateMachine{0};
public:
	AnimationStateMachineComponent(const ConstructorArgs<AnimationStateMachineComponent>&);
	AnimationStateMachineComponent(DAnimationStateMachine);
	~AnimationStateMachineComponent();
public:
	virtual void* GetAttributePtr(AttributeIdType) override;
	virtual void OnAttributeChange(AttributeIdType, void* newValue, AttributeType typeHint) override;
public:
	void Setup();
	void Tick(float deltaTime);
public:
	void AttachToEntity(EntityRef entity)
	{
		m_animationStateMachine.AttachTo(entity);
	}
public:
	template <ParameterType ParameterT>
	bool TryGetParameterIndexWithName(const stringType& name, size_t& out)
	{
		return m_animationStateMachine.TryGetParameterIndexWithName<ParameterT>(name, out);
	}

	template <ParameterType ParameterT, class ParameterValueType>
	void SetParameterValue(size_t parameterIndex, ParameterValueType value)
	{
		m_animationStateMachine.SetParameterValue<ParameterT>(parameterIndex, value);
	}
private:
	DAnimationStateMachine m_animationStateMachine;
};

class AnimationStateMachineComponentFormGenerator : public ComponentFormGenerator
{
public:
	~AnimationStateMachineComponentFormGenerator() = default;
private:
	 AnimationStateMachineComponentFormGenerator()
		:
		ComponentFormGenerator
		(
			{
				ComponentId::GetId<AnimationStateMachineComponent>(),
				"Animation State Machine Component",
				false,
				sizeof(AnimationStateMachineComponent),
				sizeof(ConstructorArgs<AnimationStateMachineComponent>),
				{{AttributeName("Animation State Machine"), AttributeType::AnimationStateMachine, AnimationStateMachineComponent::a_animationStateMachine}},
				[](void* address, const void* args) -> void
				{
					new (address) AnimationStateMachineComponent(*static_cast<const ConstructorArgs<AnimationStateMachineComponent>*>(args));
				},
				[](void* componentAddress) -> void
				{
					static_cast<AnimationStateMachineComponent*>(componentAddress)->~AnimationStateMachineComponent();
				},
				&m_defaultArgs
			}
		)
	{}
private:
	 ConstructorArgs<AnimationStateMachineComponent> m_defaultArgs;
private:
	 static AnimationStateMachineComponentFormGenerator s_generator;
};

template <>
class ComponentRef<AnimationStateMachineComponent>
{
public:
	using stringType = std::string;
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
	bool IsValid()
	{
		if (m_lockData == nullptr)
		{
			return false;
		}
		return m_internalSceneRef.IsValid() && m_entity.IsValid() && m_internalSceneRef->GetAsset().GetRegistry().HaveComponents<AnimationStateMachineComponent>(m_entity);
	}

	void Setup()
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
		DASSERT_E(registry.HaveComponents<AnimationStateMachineComponent>(m_entity));
		AnimationStateMachineComponent& component(registry.GetComponents<AnimationStateMachineComponent>(m_entity));
		component.Setup();
	}

	void Tick(float deltaTime)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
		DASSERT_E(registry.HaveComponents<AnimationStateMachineComponent>(m_entity));
		AnimationStateMachineComponent& component(registry.GetComponents<AnimationStateMachineComponent>(m_entity));
		component.Tick(deltaTime);
	}
	
	void AttachToEntity(EntityRef entity)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
		DASSERT_E(registry.HaveComponents<AnimationStateMachineComponent>(m_entity));
		AnimationStateMachineComponent& component(registry.GetComponents<AnimationStateMachineComponent>(m_entity));
		component.AttachToEntity(entity);
	}
public:
	template <ParameterType ParameterT>
	bool TryGetParameterIndexWithName(const stringType& name, size_t& out)
	{
		DASSERT_E(IsValid());
		Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
		DASSERT_E(registry.HaveComponents<AnimationStateMachineComponent>(m_entity));
		AnimationStateMachineComponent& component(registry.GetComponents<AnimationStateMachineComponent>(m_entity));
		return component.TryGetParameterIndexWithName<ParameterT>(name, out);
	}

	template <ParameterType ParameterT, class ParameterValueType>
	void SetParameterValue(size_t parameterIndex, ParameterValueType value)
	{
		DASSERT_E(IsValid());
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		Registry& registry(m_internalSceneRef->GetAsset().GetRegistry());
		DASSERT_E(registry.HaveComponents<AnimationStateMachineComponent>(m_entity));
		AnimationStateMachineComponent& component(registry.GetComponents<AnimationStateMachineComponent>(m_entity));
		component.SetParameterValue<ParameterT>(parameterIndex, value);
	}
private:
	Entity m_entity;
	InternalSceneRefType m_internalSceneRef;
	LockData* m_lockData;
};

}
