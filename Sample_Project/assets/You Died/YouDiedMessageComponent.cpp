#include "YouDiedMessageComponent.h"



namespace Game
{

YouDiedMessageComponentScriptComponentFormGenerator YouDiedMessageComponentScriptComponentFormGenerator::s_generator;

void YouDiedMessageComponent::Start()
{
	auto [asmComponent, spriteComponent] = m_entityRef.GetComponents<DCore::AnimationStateMachineComponent, DCore::SpriteComponent>();
	m_asmComponent = asmComponent;
	m_spriteComponent = spriteComponent;
	DASSERT_E(m_asmComponent.IsValid() && m_spriteComponent.IsValid());
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Display", m_displayAnimationParameter));
	m_spriteComponent.SetEnabled(false);
	m_entityRef.IterateOnChildren(
		[&](DCore::EntityRef entity) -> bool
		{
			DCore::DString name;
			entity.GetName(name);
			if (name == "Background")
			{
				m_backgroundSpriteComponent = entity.GetComponents<DCore::SpriteComponent>();
				DASSERT_E(m_backgroundSpriteComponent.IsValid());
				m_backgroundSpriteComponent.SetEnabled(false);
				return false;
			}
			if (name == "Press R To Restart")
			{
				m_pressRToRestartSpriteComponent = entity.GetComponents<DCore::SpriteComponent>();
				DASSERT_E(m_pressRToRestartSpriteComponent.IsValid());
				m_pressRToRestartSpriteComponent.SetEnabled(false);
				return false;
			}
			return false;
		});
}

void YouDiedMessageComponent::OnAnimationEvent(size_t metachannelId)
{
	constexpr size_t displayAnimationEvent(0);
	switch (metachannelId)
	{
	case displayAnimationEvent:
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_displayAnimationParameter, DCore::LogicParameter{ false });
		m_spriteComponent.SetEnabled(false);
		m_pressRToRestartSpriteComponent.SetEnabled(true);
		return;
	default:
		return;
	}
}

void YouDiedMessageComponent::Display()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_displayAnimationParameter, DCore::LogicParameter{ true });
	m_spriteComponent.SetEnabled(true);
	m_backgroundSpriteComponent.SetEnabled(true);
}

}