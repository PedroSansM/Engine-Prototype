#include "IrisTransitionComponent.h"



namespace Game
{

IrisTransitionComponentScriptComponentFormGenerator IrisTransitionComponentScriptComponentFormGenerator::s_generator;

void IrisTransitionComponent::Start()
{
	m_spriteComponent = m_entityRef.GetComponents<DCore::SpriteComponent>();
	DASSERT_E(m_spriteComponent.IsValid());
	m_spriteComponent.SetSpriteIndex(0);
}

void IrisTransitionComponent::OnMetachannelEvent(size_t eventId)
{
	constexpr size_t transitionEnd(0);
	switch (eventId)
	{
	case transitionEnd:
		m_spriteComponent.SetEnabled(false);
		return;
	default:
		return;
	}
}

}