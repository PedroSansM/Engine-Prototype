#include "SeedComponent.h"
#include "SeedEnemies.h"
#include "ChomperComponent.h"
#include "VenusComponent.h"



namespace Game
{

SeedComponentScriptComponentFormGenerator SeedComponentScriptComponentFormGenerator::s_generator;

SeedComponent::SeedComponent(const DCore::ConstructorArgs<SeedComponent>& args)
	:
	m_seedFallVelocity(args.SeedFallVelocity)
{}

SeedComponent::~SeedComponent()
{
	m_runtime->RemoveFromOnCollisionBegin(m_seedBoxColliderOnCollisionBeginRegistrationIndex, m_seedBoxColliderId);
}

void* SeedComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_seedFallVelocity:
		return &m_seedFallVelocity;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void SeedComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_seedFallVelocity:
		m_seedFallVelocity = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	default:
		return;
	}
}

void SeedComponent::Awake()
{
	auto [spriteComponent, asmComponent] = m_entityRef.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent>();
	m_spriteComponent = spriteComponent;
	m_asmComponent = asmComponent;
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Plant", m_plantAnimationParameter));
	DCore::EntityRef parent;
	DASSERT_K(m_entityRef.TryGetParent(parent));
	for (size_t i(0); i < SeedEnemies::NumberOfSeedEnemies; i++)
	{
		const DCore::ComponentIdType componentId(SeedEnemies::EnemiesIds[i]);
		if (parent.HaveComponent(componentId))
		{
			m_burstable = parent.GetComponent(componentId);
			m_seedBoxColliderComponent = parent.GetComponents<DCore::BoxColliderComponent>();
			m_seedBoxColliderId = m_seedBoxColliderComponent.GetBodyId();
			break;
		}
	}
	m_seedBoxColliderOnCollisionBeginRegistrationIndex = m_runtime->RegisterToOnCollisionBegin(GenerateComponentRefConstructorArgs(), m_seedBoxColliderId);
	DASSERT_E(m_burstable.IsValid() && m_spriteComponent.IsValid() && m_asmComponent.IsValid() && m_seedBoxColliderComponent.IsValid());
	m_spriteComponent.SetEnabled(false);
	m_seedBoxColliderComponent.SetEnabled(false);
}

void SeedComponent::OnCollisionBegin(DCore::EntityRef entity)
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_plantAnimationParameter, DCore::LogicParameter{ true });
}

void SeedComponent::OnAnimationEvent(size_t metachannelId)
{
	constexpr size_t plantAnimationEventId(0);
	switch (metachannelId)
	{
	case plantAnimationEventId:
	{
		m_spriteComponent.SetEnabled(false);
		m_seedBoxColliderComponent.SetEnabled(false);
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_plantAnimationParameter, DCore::LogicParameter{ false });
		const DCore::ComponentIdType componentId(m_burstable.GetComponentId());
		if (componentId == DCore::ComponentId::GetId<ChomperComponent>())
		{
			ChomperComponent* chomperComponent(static_cast<ChomperComponent*>(m_burstable.GetRawComponent()));
			chomperComponent->HandleBurstBeginResponse();
			return;
		}
		if (componentId == DCore::ComponentId::GetId<VenusComponent>())
		{
			VenusComponent* venusComponent(static_cast<VenusComponent*>(m_burstable.GetRawComponent()));
			venusComponent->HandleBurstBeginResponse();
			return;
		}
		return;
	}
	default:
		return;
	}
}

void SeedComponent::BeginFall()
{
	m_spriteComponent.SetEnabled(true);
	m_seedBoxColliderComponent.SetEnabled(true);
	m_seedBoxColliderComponent.SetLinearVelocity({ 0.0f, -m_seedFallVelocity });
}
 
}