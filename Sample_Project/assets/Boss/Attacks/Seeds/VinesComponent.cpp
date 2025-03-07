#include "VinesComponent.h"
#include "SeedEnemies.h"
#include "VenusComponent.h"



namespace Game
{

VinesComponentScriptComponentFormGenerator VinesComponentScriptComponentFormGenerator::s_generator;

VinesComponent::VinesComponent(const DCore::ConstructorArgs<VinesComponent>& args)
{}

void VinesComponent::Start()
{
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
	auto [spriteComponent, asmComponent] = m_entityRef.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent>();
	m_spriteComponent = spriteComponent;
	m_asmComponent = asmComponent;
	DASSERT_E(m_spriteComponent.IsValid() && m_asmComponent.IsValid());
	m_spriteComponent.SetEnabled(false);
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Grow", m_growAnimationParameter));
}

void VinesComponent::OnAnimationEvent(size_t metachannelId)
{
	constexpr size_t showCreatureAnimationEvent(0);
	constexpr size_t releaseAnimationEvent(1);
	constexpr size_t growEndAnimationEvent(2);
	switch (metachannelId)
	{
	case showCreatureAnimationEvent:
	{
		const DCore::ComponentIdType componentId(m_burstable.GetComponentId());
		if (componentId == DCore::ComponentId::GetId<VenusComponent>())
		{
			VenusComponent* venusComponent(static_cast<VenusComponent*>(m_burstable.GetRawComponent()));
			venusComponent->HandleShowCreatureResponse();
			return;
		}
		return;
	}
	case releaseAnimationEvent:
	{
		const DCore::ComponentIdType componentId(m_burstable.GetComponentId());
		if (componentId == DCore::ComponentId::GetId<VenusComponent>())
		{
			VenusComponent* venusComponent(static_cast<VenusComponent*>(m_burstable.GetRawComponent()));
			venusComponent->HandleVinesReleaseResponse();
			return;
		}
		return;
	}
	case growEndAnimationEvent:
		m_spriteComponent.SetEnabled(false);
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_growAnimationParameter, DCore::LogicParameter{ false });
		return;
	default:
		break;
	}
}

void VinesComponent::BeginGrow()
{
	m_spriteComponent.SetEnabled(true);
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_growAnimationParameter, DCore::LogicParameter{ true });
}

}