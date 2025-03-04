#include "PlayerHealthComponent.h"



namespace Game
{

PlayerHealthComponentScriptComponentFormGenerator PlayerHealthComponentScriptComponentFormGenerator::s_generator;

static constexpr float s_blinkPeriod(0.06f);
	 
void PlayerHealthComponent::Start()
{
	m_currentHeath = 3;
	m_currentBlinkPeriod = s_blinkPeriod;
	m_spriteComponent = m_entityRef.GetComponents<DCore::SpriteComponent>();
	DASSERT_E(m_spriteComponent.IsValid());
	m_spriteComponent.SetSpriteIndex(m_currentHeath + 2);
}

void PlayerHealthComponent::Update(float deltaTime)
{
	if (m_currentHeath != 1)
	{
		return;
	}
	m_currentBlinkPeriod -= deltaTime;
	if (m_currentBlinkPeriod <= 0.0f)
	{
		m_currentBlinkPeriod = s_blinkPeriod;
		if (m_spriteComponent.GetSpriteIndex() == 2)
		{
			m_spriteComponent.SetSpriteIndex(3);
		}
		else
		{
			m_spriteComponent.SetSpriteIndex(2);
		}
	}
}

void PlayerHealthComponent::DecreaseHealth()
{
	m_currentHeath -= 1;
	if (m_currentHeath == 0)
	{
		m_spriteComponent.SetSpriteIndex(0);
		return;
	}
	m_spriteComponent.SetSpriteIndex(m_currentHeath + 2);
}

}