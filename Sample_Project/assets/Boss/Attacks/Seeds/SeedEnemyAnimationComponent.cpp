#include "SeedEnemyAnimationComponent.h"
#include "SeedEnemies.h"
#include "ChomperComponent.h"
#include "VenusComponent.h"



namespace Game
{

SeedEnemyAnimationComponentScriptComponentFormGenerator SeedEnemyAnimationComponentScriptComponentFormGenerator::s_generator;

void SeedEnemyAnimationComponent::Start()
{
	DCore::EntityRef parent;
	DASSERT_K(m_entityRef.TryGetParent(parent));
	for (size_t i(0); i < SeedEnemies::NumberOfSeedEnemies; i++)
	{
		const DCore::ComponentIdType componentId(SeedEnemies::EnemiesIds[i]);
		if (parent.HaveComponent(componentId))
		{
			m_burstable = parent.GetComponent(componentId);
			return;
		}
	}
}

void SeedEnemyAnimationComponent::OnAnimationEvent(size_t metachannelId)
{
	constexpr size_t deathEndAnimationEvent(0);
	switch (metachannelId)
	{
	case deathEndAnimationEvent:
	{
		if (m_burstable.GetComponentId() == DCore::ComponentId::GetId<ChomperComponent>())
		{
			ChomperComponent* chomperComponent(static_cast<ChomperComponent*>(m_burstable.GetRawComponent()));
		chomperComponent->HandleDeathEndResponse();
			return;
		}
		if (m_burstable.GetComponentId() == DCore::ComponentId::GetId<VenusComponent>())
		{
			VenusComponent* venusComponent(static_cast<VenusComponent*>(m_burstable.GetRawComponent()));
			venusComponent->HandleDeathEndResponse();
			return;
		}
		return;
	}
	default:
		return;
	}
}

}