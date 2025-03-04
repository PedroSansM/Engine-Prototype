#include "HourglassComponent.h"



namespace Game
{

HourglassComponentScriptComponentFormGenerator HourglassComponentScriptComponentFormGenerator::s_generator;

void HourglassComponent::Awake()
{
	auto [asmComponent, spriteComponent] = m_entityRef.GetComponents<DCore::AnimationStateMachineComponent, DCore::SpriteComponent>();
	m_asmComponent = asmComponent;
	m_spriteComponent = spriteComponent;
	DASSERT_E(m_asmComponent.IsValid() && m_spriteComponent.IsValid());
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Animate", m_animateAnimationParameter));
	m_spriteComponent.SetEnabled(false);
	m_entityRef.IterateOnChildren(
		[&](DCore::EntityRef child) -> bool
		{
			m_loadingScreenSpriteComponent = child.GetComponents<DCore::SpriteComponent>();
			DASSERT_E(m_loadingScreenSpriteComponent.IsValid());
			m_loadingScreenSpriteComponent.SetEnabled(false);
			return true;
		});
}

void HourglassComponent::DisplayLoadingScreen()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_animateAnimationParameter, DCore::LogicParameter{ true });
	m_spriteComponent.SetEnabled(true);
	m_loadingScreenSpriteComponent.SetEnabled(true);
}

void HourglassComponent::HideLoadingScreen()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_animateAnimationParameter, DCore::LogicParameter{ false });
	m_spriteComponent.SetEnabled(false);
	m_loadingScreenSpriteComponent.SetEnabled(false);
}

}
