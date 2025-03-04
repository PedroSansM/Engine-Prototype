#include "PlatformComponent.h"
#include "glm/gtc/constants.hpp"



namespace Game
{

PlatformComponentScriptComponentFormGenerator PlatformComponentScriptComponentFormGenerator::s_generator;

PlatformComponent::PlatformComponent(const DCore::ConstructorArgs<PlatformComponent>& args)
	:
	m_initialCoefficient(args.InitialCoefficient),
	m_altitudeVariation(args.AltitudeVariation),
	m_movementPeriod(args.MovementPeriod),
	m_minimalAltitude(args.MinimalAltitude),
	m_transitionVelocity(args.TransitionVelocity)
{}

void* PlatformComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_initialCoefficientId:
		return &m_initialCoefficient;
	case a_altitudeVariationId:
		return &m_altitudeVariation;
	case a_movementPeriodId:
		return &m_movementPeriod;
	case a_minimalAltitudeId:
		return &m_minimalAltitude;
	case a_transitionVelocityId:
		return &m_transitionVelocity;
	default:
		break;
	}
	return nullptr;
}

void PlatformComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_initialCoefficientId:
		m_initialCoefficient = std::min(std::max(*static_cast<float*>(newValue), 0.0f), 1.0f);
		return;
	case a_altitudeVariationId:
		m_altitudeVariation = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_movementPeriodId:
		m_movementPeriod = std::max(*static_cast<float*>(newValue), 0.1f);
		return;
	case a_minimalAltitudeId:
		m_minimalAltitude = *static_cast<float*>(newValue);
		return;
	case a_transitionVelocityId:
		m_transitionVelocity = std::max(*static_cast<float*>(newValue), 0.1f);
		return;
	default:
		return;
	}
}

void PlatformComponent::Start()
{
	m_state = PlatformState::NotColliding;
	m_currentCoefficient = m_initialCoefficient;
	m_halfAltitudeVariation = m_altitudeVariation / 2.0f;
	m_translationOffset = (m_altitudeVariation + 2 * m_minimalAltitude) / 2.0f;
	m_transform = m_entityRef.GetComponents<DCore::TransformComponent>();
}

void PlatformComponent::PhysicsUpdate(float physicsDeltaTime)
{
	switch (m_state)
	{
	case PlatformState::NotColliding:
		HandleNotCollidingState(physicsDeltaTime);
		return;
	case PlatformState::Transition:
		if (m_currentCoefficient < 0.25f)
		{
			m_currentCoefficient = 0.5f - m_currentCoefficient;
		}
		else if (m_currentCoefficient > 0.75f)
		{
			m_currentCoefficient = 1.5f - m_currentCoefficient;
		}
		HandleTransitionState(physicsDeltaTime);
		return;
	}
}

void PlatformComponent::HandleNotCollidingState(float deltaTime)
{
	const float currentPositionY(m_halfAltitudeVariation * glm::sin(glm::two_pi<float>() * m_currentCoefficient) + m_translationOffset);
	m_currentCoefficient += deltaTime / m_movementPeriod;
	if (m_currentCoefficient >= 1.0f)
	{
		m_currentCoefficient = 0.0f;
	}
	m_transform.SetTranslation({m_transform.GetTranslation().x, currentPositionY, 0.0f});
}

void PlatformComponent::HandleTransitionState(float deltaTime)
{
	constexpr float stoppingDistance(0.01f);
	if (glm::abs(m_currentCoefficient - 0.75f) <= stoppingDistance)
	{
		m_transform.SetTranslation({m_transform.GetTranslation().x, m_minimalAltitude, 0.0f});
		return;
	}
	const float currentPositionY(m_halfAltitudeVariation * glm::sin(glm::two_pi<float>() * m_currentCoefficient) + m_translationOffset);
	m_transform.SetTranslation({m_transform.GetTranslation().x, currentPositionY, 0.0f});
	m_currentCoefficient += m_transitionVelocity * deltaTime;
}

}
