#include "ChomperComponent.h"
#include "SeedComponent.h"
#include "BurstComponent.h"
#include "PlayerShotComponent.h"

#include "Log.h"



namespace Game
{

ChomperComponentScriptComponentFormGenerator ChomperComponentScriptComponentFormGenerator::s_generator;

ChomperComponent::ChomperComponent(const DCore::ConstructorArgs<ChomperComponent>& args)
	:
	m_lifeTime(args.LifeTime),
	m_health(args.Health),
	m_defeatedSound(args.DefeatedSound)
{}

ChomperComponent::~ChomperComponent()
{
	m_runtime->RemoveFromOnOverlapBegin(m_shotOverlapBeginRegistrationIndex, m_chomperHurtBoxBodyId);
}

void* ChomperComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_lifeTime:
		return &m_lifeTime;
	case a_health:
		return &m_health;
	case a_defeatedSound:
		return &m_defeatedSound;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void ChomperComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_lifeTime:
		m_lifeTime = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_health:
		m_health = *static_cast<DCore::DUInt*>(newValue);
		return;
	case a_defeatedSound:
		m_defeatedSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;
	default:
		return;
	}
}

void ChomperComponent::Start()
{
	DASSERT_K(m_defeatedSound.Setup());
	m_currentLifeTime = 0.0f;
	m_currentHealth = m_health;
	m_entityRef.IterateOnChildren
	(
		[&](DCore::EntityRef entity) -> bool
		{
			DCore::DString name;
			entity.GetName(name);
			if (name == "Seed")
			{
				m_seedComponent = entity.GetComponent(DCore::ComponentId::GetId<SeedComponent>());
				DASSERT_E(m_seedComponent.IsValid());
				return false;
			}
			if (name == "Burst")
			{
				m_burstComponent = entity.GetComponent(DCore::ComponentId::GetId<BurstComponent>());
				DASSERT_E(m_burstComponent.IsValid());
				return false;
			}
			if (name == "Chomper")
			{
				auto [spriteComponent, boxColliderComponent, asmComponent, transformComponent] = entity.GetComponents<DCore::SpriteComponent, DCore::BoxColliderComponent, DCore::AnimationStateMachineComponent, DCore::TransformComponent>();
				m_chomperSpriteComponent = spriteComponent;
				m_chomperBoxColliderComponent = boxColliderComponent;
				m_chomperAsmComponent = asmComponent;
				m_chomperTransformComponent = transformComponent;
				DASSERT_E(m_chomperSpriteComponent.IsValid() && m_chomperBoxColliderComponent.IsValid() && m_chomperAsmComponent.IsValid() && m_chomperTransformComponent.IsValid());
				m_chomperSpriteComponent.SetEnabled(false);
				m_chomperBoxColliderComponent.SetEnabled(false);
				DASSERT_K(m_chomperAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Bite", m_chomperBiteAnimationParameter));
				DASSERT_K(m_chomperAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Death", m_chomperDeathAnimationParameter));
				DASSERT_K(m_chomperAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Reset", m_chomperResetAnimationParameter));
				m_chomperRelativePosition = m_chomperTransformComponent.GetTranslation();
				entity.IterateOnChildren
				(
					[&](DCore::EntityRef child) -> bool
					{
						m_chomperHurtBoxComponent = child.GetComponents<DCore::BoxColliderComponent>();
						DASSERT_E(m_chomperHurtBoxComponent.IsValid());
						m_chomperHurtBoxBodyId = m_chomperHurtBoxComponent.GetBodyId();
						m_shotOverlapBeginRegistrationIndex = m_runtime->RegisterToOnOverlapBegin(GenerateComponentRefConstructorArgs(), m_chomperHurtBoxBodyId);
						m_chomperHurtBoxComponent.SetEnabled(false);
						return true;
					}
				);
				return false;
			}
			return false;
		}
	);
}

void ChomperComponent::Update(float deltaTime)
{
	if (m_currentLifeTime <= 0.0f)
	{
		return;
	}
	m_currentLifeTime -= deltaTime;
	if (m_currentLifeTime <= 0.0f)
	{
		m_chomperBoxColliderComponent.SetEnabled(false);
		m_chomperHurtBoxComponent.SetEnabled(false);
		m_chomperAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_chomperDeathAnimationParameter, DCore::TriggerParameter{true});
	}
}

void ChomperComponent::OnOverlapBegin(DCore::EntityRef entity)
{
	m_currentHealth -= 1;
	if (m_currentHealth == 0)
	{
		m_defeatedSound.Start();
		m_chomperBoxColliderComponent.SetEnabled(false);
		m_chomperHurtBoxComponent.SetEnabled(false);
		m_chomperAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_chomperDeathAnimationParameter, DCore::TriggerParameter{true});
		m_currentLifeTime = 0.0f;
	}
	DCore::ComponentRef<DCore::Component> playerShotComponentRef(entity.GetComponent(DCore::ComponentId::GetId<PlayerShotComponent>()));
	PlayerShotComponent* playerShotComponent(static_cast<PlayerShotComponent*>(playerShotComponentRef.GetRawComponent()));
	playerShotComponent->Kill();
}

void ChomperComponent::HandleBurstBeginResponse()
{
	BurstComponent* burstComponent(static_cast<BurstComponent*>(m_burstComponent.GetRawComponent()));
	burstComponent->BeginBurst();
}

void ChomperComponent::HandleBurstReleaseResponse()
{
	m_chomperSpriteComponent.SetEnabled(true);
	m_chomperBoxColliderComponent.SetEnabled(true);
	m_chomperHurtBoxComponent.SetEnabled(true);
	m_chomperTransformComponent.SetTranslation(m_chomperRelativePosition);
	m_chomperAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_chomperBiteAnimationParameter, DCore::TriggerParameter{ true });
	m_currentLifeTime = m_lifeTime;
	m_currentHealth = m_health;
}

void ChomperComponent::HandleDeathEndResponse()
{
	m_chomperSpriteComponent.SetEnabled(false);
	m_chomperAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_chomperResetAnimationParameter, DCore::TriggerParameter{ true });
}

}