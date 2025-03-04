#include "BoomerangComponent.h"



namespace Game
{

BoomerangComponentScriptComponentFormGenerator BoomerangComponentScriptComponentFormGenerator::s_generator;

BoomerangComponent::BoomerangComponent(const DCore::ConstructorArgs<BoomerangComponent>& args)
	:
	m_sound(args.Sound)
{}

void* BoomerangComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_sound:
		return &m_sound;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void BoomerangComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_sound:
		m_sound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;
	default:
		return;
	}
}

void BoomerangComponent::Start()
{
	DASSERT_K(m_sound.Setup());
	auto [transformComponent, boxColliderComponent, spriteComponent] = m_entityRef.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent, DCore::SpriteComponent>();
	m_transformComponent = transformComponent;
	m_boxColliderComponent = boxColliderComponent;
	m_spriteComponent = spriteComponent;
	m_boxColliderComponent.SetEnabled(false);
	m_spriteComponent.SetEnabled(false);
	m_initialPosition = m_transformComponent.GetTranslation();
	m_currentHalfLifeTime = 0.0f;
	m_currentTimeToLaunch = 0.0f;
	m_isFree = true;
}

void BoomerangComponent::Update(float deltaTime)
{
	if (m_currentTimeToLaunch > 0.0f)
	{
		m_currentTimeToLaunch -= deltaTime;
		if (m_currentTimeToLaunch <= 0.0f)
		{
			Launch();
		}
	}
	if (m_currentHalfLifeTime <= 0.0f)
	{
		return;
	}
	m_currentHalfLifeTime -= deltaTime;
	m_sound.Set3DAttributes(m_transformComponent.GetTranslation(), { m_velocity * (m_halfWayCompleted ? 1.0f : -1.0f), 0.0f, 0.0f});
	if (m_currentHalfLifeTime <= 0.0f)
	{
		if (!m_halfWayCompleted)
		{
			m_halfWayCompleted = true;
			m_currentHalfLifeTime = m_halfLifeTime + m_additionalReturnLifeTime;
			ChangeDirection();
			return;
		}
		Disable();
	}
}

void BoomerangComponent::Enable()
{
	m_currentTimeToLaunch = m_timeToLaunch;
	m_spriteComponent.SetEnabled(true);
	m_boxColliderComponent.SetEnabled(true);
	m_halfWayCompleted = false;
	m_isFree = false;
	m_sound.Start();
}

void BoomerangComponent::Launch()
{
	m_currentHalfLifeTime = m_halfLifeTime;
	m_boxColliderComponent.SetLinearVelocity({-m_velocity, 0.0f});
}

void BoomerangComponent::ChangeDirection()
{
	m_boxColliderComponent.SetLinearVelocity({m_velocity, 0.0f});
	m_transformComponent.SetTranslation({m_transformComponent.GetTranslation().x, m_lowTranslationY, 0.0f});
}

void BoomerangComponent::Disable()
{
	m_boxColliderComponent.SetLinearVelocity({0.0f, 0.0f});
	m_boxColliderComponent.SetEnabled(false);
	m_spriteComponent.SetEnabled(false);
	m_transformComponent.SetTranslation(m_initialPosition);
	m_isFree = true;
	m_sound.Stop();
}

}
