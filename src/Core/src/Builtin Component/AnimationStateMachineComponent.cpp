#include "AnimationStateMachineComponent.h"
#include "SerializationTypes.h"
#include "TemplateUtils.h"
#include "ComponentForm.h"
#include "ScriptComponent.h"



namespace DCore
{

AnimationStateMachineComponent::AnimationStateMachineComponent(const ConstructorArgs<AnimationStateMachineComponent>& args)
	:
	m_animationStateMachine(args.AnimationStateMachine)
{}

AnimationStateMachineComponent::AnimationStateMachineComponent(DAnimationStateMachine animationStateMachine)
	:
	m_animationStateMachine(animationStateMachine)
{}

AnimationStateMachineComponent::~AnimationStateMachineComponent()
{
	m_animationStateMachine.Unload();
}

void AnimationStateMachineComponent::Setup()
{
	m_animationStateMachine.Setup();
}

void AnimationStateMachineComponent::Tick(float deltaTime)
{
	if (m_animationStateMachine.IsValid())
	{
		m_animationStateMachine.Tick(
			deltaTime, 
			[&](EntityRef entity, size_t metachannelId)
			{
				const ComponentForms::scriptComponentIdContainerType& scriptComponentIds(ComponentForms::Get().GetScriptComponentIds());
				for (ComponentIdType componentId : scriptComponentIds)
				{
					if (!entity.HaveComponents(&componentId, 1))
					{
						continue;
					}
					entity.GetComponents(
						&componentId, 1,
						[&](ComponentRef<Component> component) -> void
						{
							static_cast<ScriptComponent*>(component.GetRawComponent())->OnMetachannelEvent(metachannelId);
						});
				}
			});
	}
}

void* AnimationStateMachineComponent::GetAttributePtr(AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_animationStateMachine:
		return &m_animationStateMachine;
	default:
		return nullptr;
	}
}

void AnimationStateMachineComponent::OnAttributeChange(AttributeIdType attributeId, void* newValue, AttributeType)
{
	switch (attributeId)
	{
	case a_animationStateMachine:
		m_animationStateMachine = *static_cast<DAnimationStateMachine*>(newValue);
		return;
	default:
		DASSERT_E(false);
		return;
	}
}

AnimationStateMachineComponentFormGenerator AnimationStateMachineComponentFormGenerator::s_generator;

}
