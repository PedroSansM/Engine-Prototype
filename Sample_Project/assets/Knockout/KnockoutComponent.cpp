#include "KnockoutComponent.h"



namespace Game
{

KnockoutComponentScriptComponentFormGenerator KnockoutComponentScriptComponentFormGenerator::s_generator;
 
void KnockoutComponent::Start()
{
	auto [asmComponent, spriteComponent] = m_entityRef.GetComponents<DCore::AnimationStateMachineComponent, DCore::SpriteComponent>();
	m_asmComponent = asmComponent;
	m_spriteComponent = spriteComponent;
	DASSERT_E(m_asmComponent.IsValid() && m_spriteComponent.IsValid());
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Display", m_displayAnimationParameter));
	m_spriteComponent.SetEnabled(false);
}

void KnockoutComponent::OnMetachannelEvent(size_t metachannelId)
{
	constexpr size_t displayEnd(0);
	switch (displayEnd)
	{
	case displayEnd:
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_displayAnimationParameter, DCore::LogicParameter{ false });
		m_spriteComponent.SetEnabled(false);
		return;
	default:
		return;
	}
}

void KnockoutComponent::DisplayKnockoutMessage()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_displayAnimationParameter, DCore::LogicParameter{ true });
	m_spriteComponent.SetEnabled(true);
}

}