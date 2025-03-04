#include "BurstComponent.h"
#include "SeedEnemies.h"
#include "ChomperComponent.h"
#include "VenusComponent.h"



namespace Game
{

BurstComponentScriptComponentFormGenerator BurstComponentScriptComponentFormGenerator::s_generator;

void BurstComponent::Start()
{
	auto [spriteComponent, asmComponent] = m_entityRef.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent>();
	m_spriteComponent = spriteComponent;
	m_asmComponent = asmComponent;
	DCore::EntityRef parent;
	DASSERT_K(m_entityRef.TryGetParent(parent));
	for (size_t i(0); i < SeedEnemies::NumberOfSeedEnemies; i++)
	{
		const DCore::ComponentIdType componentId(SeedEnemies::EnemiesIds[i]);
		if (parent.HaveComponent(componentId))
		{
			m_burstable = parent.GetComponent(componentId);
			break;
		}
	}
	DASSERT_E(m_burstable.IsValid() && m_spriteComponent.IsValid() && m_asmComponent.IsValid());
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Burst", m_burstAnimationParameter));
	m_spriteComponent.SetEnabled(false);
}

void BurstComponent::OnMetachannelEvent(size_t metachannelId)
{
	constexpr size_t burstReleaseAnimationEventId(0);
	constexpr size_t burstEndAnimationEventId(1);
	switch (metachannelId)
	{
	case burstReleaseAnimationEventId:
	{
		const DCore::ComponentIdType componentId(m_burstable.GetComponentId());
		if (componentId == DCore::ComponentId::GetId<ChomperComponent>())
		{
			ChomperComponent* chomperComponent(static_cast<ChomperComponent*>(m_burstable.GetRawComponent()));
			chomperComponent->HandleBurstReleaseResponse();
			return;
		}
		if (componentId == DCore::ComponentId::GetId<VenusComponent>())
		{
			VenusComponent* venusComponent(static_cast<VenusComponent*>(m_burstable.GetRawComponent()));
			venusComponent->HandleBurstReleaseResponse();
			return;
		}
		return;
	}
	case burstEndAnimationEventId:
	{
		m_spriteComponent.SetEnabled(false);
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_burstAnimationParameter, DCore::LogicParameter{ false });
		return;
	}
	default:
		return;
	}
}

void BurstComponent::BeginBurst()
{
	m_spriteComponent.SetEnabled(true);
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_burstAnimationParameter, DCore::LogicParameter{ true });
}

}