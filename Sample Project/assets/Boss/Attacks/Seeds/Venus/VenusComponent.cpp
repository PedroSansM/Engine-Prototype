#include "VenusComponent.h"
#include "SeedComponent.h"
#include "BurstComponent.h"
#include "VinesComponent.h"
#include "PlayerComponent.h"
#include "PlayerShotComponent.h"

#include "Log.h"

#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"



namespace Game
{

VenusComponentScriptComponentFormGenerator VenusComponentScriptComponentFormGenerator::s_generator;

VenusComponent::VenusComponent(const DCore::ConstructorArgs<VenusComponent>& args)
	:
	m_venusLinearVelocity(args.VenusLinearVelocity),
	m_venusAngularVelocity(args.VenusAngularVelocity),
	m_timerPursuingPlayer(args.TimePursuingPlayer),
	m_lifeTime(args.LifeTime),
	m_health(args.Health),
	m_defeatedSound(args.DefeatedSound)
{}

VenusComponent::~VenusComponent()
{
	m_runtime->RemoveFromOnOverlapBegin(m_shotOverlapBeginRegistrationIndex, m_venusHurtBoxBodyId);
}

void* VenusComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_venusLinearVelocity:
		return &m_venusLinearVelocity;
	case a_venusAngularVelocity:
		return &m_venusAngularVelocity;
	case a_timePursuingPlayer:
		return &m_timerPursuingPlayer;
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

void VenusComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_venusLinearVelocity:
		m_venusLinearVelocity = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_venusAngularVelocity:
		m_venusAngularVelocity = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_timePursuingPlayer:
		m_timerPursuingPlayer = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
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

void VenusComponent::Start()
{
	DASSERT_K(m_defeatedSound.Setup());
	m_isBorn = false;
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
			if (name == "Vines")
			{
				m_vinesComponent = entity.GetComponent(DCore::ComponentId::GetId<VinesComponent>());
				DASSERT_E(m_vinesComponent.IsValid());
				return false;
			}
			if (name == "Venus")
			{
				m_venusEntity = entity;
				auto [spriteComponent, asmComponent, boxColliderComponent, transformComponent] = entity.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent, DCore::BoxColliderComponent, DCore::TransformComponent>();
				m_venusSpriteComponent = spriteComponent;
				m_venusAsmComponent = asmComponent;
				m_venusBoxColliderComponent = boxColliderComponent;
				m_venusTransformComponent = transformComponent;
				DASSERT_E(m_venusSpriteComponent.IsValid() && m_venusAsmComponent.IsValid() && m_venusBoxColliderComponent.IsValid() && m_venusTransformComponent.IsValid());
				m_venusSpriteComponent.SetEnabled(false);
				DASSERT_K(m_venusAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Birth", m_venusBirthAnimationParameter));
				DASSERT_K(m_venusAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Bite", m_venusBiteAnimationParameter));
				DASSERT_K(m_venusAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Death", m_venusDeathAnimationParameter));
				DASSERT_K(m_venusAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Reset", m_venusResetAnimationPanemeter));
				m_venusBoxColliderComponent.SetEnabled(false);
				entity.IterateOnChildren
				(
					[&](DCore::EntityRef child) -> bool
					{
						m_venusHurtBoxComponent = child.GetComponents<DCore::BoxColliderComponent>();
						DASSERT_E(m_venusHurtBoxComponent.IsValid());
						m_venusHurtBoxBodyId = m_venusHurtBoxComponent.GetBodyId();
						m_shotOverlapBeginRegistrationIndex = m_runtime->RegisterToOnOverlapBegin(GenerateComponentRefConstructorArgs(), m_venusHurtBoxBodyId);
						m_venusHurtBoxComponent.SetEnabled(false);
						return true;
					}
				);
				return false;
			}
			return false;
		}
	);
	const DCore::ComponentIdType componentId(DCore::ComponentId::GetId<PlayerComponent>());
	IterateOnEntitiesWithComponents
	(
		&componentId, 1,
		[&](DCore::Entity entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			DCore::EntityRef entityRef(entity, m_entityRef.GetSceneRef());
			m_playerTransformComponent = entityRef.GetComponents<DCore::TransformComponent>();
			DASSERT_E(m_playerTransformComponent.IsValid());
			return true;
		}
	);
}

void VenusComponent::PhysicsUpdate(float deltaTime)
{
	if (!m_isBorn)
	{
		return;
	}
	if (m_currentTimePursuingPlayer > 0.0f)
	{
		const DCore::DVec2 venusPos(m_venusEntity.GetWorldTranslation());
		const DCore::DVec2 scale(m_venusTransformComponent.GetScale());
		const float limitedRotation(m_venusTransformComponent.GetRotation());
		float freeRotation(limitedRotation);
		if (scale.x < 0.0f)
		{
			freeRotation += 180.0f;
		}
		const DCore::DVec2 playerPos(m_playerTransformComponent.GetTranslation());
		DCore::DVec2 orientation(glm::rotate(DCore::DVec2(1.0f, 0.0f), glm::radians(freeRotation)) * -1.0f);
		const DCore::DVec2 toPlayerDir(glm::normalize(playerPos - venusPos));
		const float differentialAngle(glm::orientedAngle(orientation, toPlayerDir));
		const DCore::DVec2 deltaPosition(orientation * m_venusLinearVelocity * deltaTime);
		const float deltaRotation(m_venusAngularVelocity * deltaTime * (differentialAngle >= 0.0f ? 1.0f : -1.0f));
		m_venusEntity.SetWorldTranslation(DCore::DVec3(venusPos.x + deltaPosition.x, venusPos.y + deltaPosition.y, 0.0f));
		m_venusTransformComponent.SetRotation(freeRotation + deltaRotation);
		const float resultantLimitedRotation(limitedRotation + deltaRotation);
		if (resultantLimitedRotation >= 90.0f || resultantLimitedRotation <= -90.0f)
		{
			m_venusTransformComponent.SetScale({ -scale.x, -scale.y });
		}
		m_currentTimePursuingPlayer -= deltaTime;
		return;
	}
	const DCore::DVec2 scale(m_venusTransformComponent.GetScale());
	const float limitedRotation(m_venusTransformComponent.GetRotation());
	float freeRotation(limitedRotation);
	if (scale.x < 0.0f)
	{
		freeRotation += 180.0f;
	}
	const DCore::DVec2 venusPos(m_venusEntity.GetWorldTranslation());
	DCore::DVec2 orientation(glm::rotate(DCore::DVec2(1.0f, 0.0f), glm::radians(freeRotation)) * -1.0f);
	const DCore::DVec2 deltaPosition(orientation * m_venusLinearVelocity * deltaTime);
	m_venusEntity.SetWorldTranslation(DCore::DVec3(venusPos.x + deltaPosition.x, venusPos.y + deltaPosition.y, 0.0f));
	m_currentLifeTime -= deltaTime;
	if (m_currentLifeTime <= 0.0f)
	{
		m_venusAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_venusDeathAnimationParameter, DCore::TriggerParameter{ true });
		m_venusBoxColliderComponent.SetEnabled(false);
		m_venusHurtBoxComponent.SetEnabled(false);
		m_isBorn = false;
	}
}

void VenusComponent::OnOverlapBegin(DCore::EntityRef entity)
{
	m_currentHealth -= 1;
	if (m_currentHealth == 0)
	{
		m_defeatedSound.Start();
		m_venusAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_venusDeathAnimationParameter, DCore::TriggerParameter{ true });
		m_venusBoxColliderComponent.SetEnabled(false);
		m_venusHurtBoxComponent.SetEnabled(false);
		m_isBorn = false;
	}
	DCore::ComponentRef<DCore::Component> playerShotComponentRef(entity.GetComponent(DCore::ComponentId::GetId<PlayerShotComponent>()));
	PlayerShotComponent* playerShotComponent(static_cast<PlayerShotComponent*>(playerShotComponentRef.GetRawComponent()));
	playerShotComponent->Kill();
}

void VenusComponent::HandleBurstBeginResponse()
{
	BurstComponent* burstComponent(static_cast<BurstComponent*>(m_burstComponent.GetRawComponent()));
	burstComponent->BeginBurst();
}

void VenusComponent::HandleBurstReleaseResponse()
{
	VinesComponent* vinesComponent(static_cast<VinesComponent*>(m_vinesComponent.GetRawComponent()));
	vinesComponent->BeginGrow();
	m_venusAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_venusBirthAnimationParameter, DCore::TriggerParameter{ true });
}

void VenusComponent::HandleShowCreatureResponse()
{
	m_venusSpriteComponent.SetEnabled(true);
}

void VenusComponent::HandleVinesReleaseResponse()
{
	m_venusAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_venusBiteAnimationParameter, DCore::TriggerParameter{ true });
	m_venusBoxColliderComponent.SetEnabled(true);
	m_venusHurtBoxComponent.SetEnabled(true);
	m_isBorn = true;
	m_currentTimePursuingPlayer = m_timerPursuingPlayer;
	m_currentLifeTime = m_lifeTime;
	m_currentHealth = m_health;
}

void VenusComponent::HandleDeathEndResponse()
{
	m_venusAsmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_venusResetAnimationPanemeter, DCore::TriggerParameter{ true });
	m_venusSpriteComponent.SetEnabled(false);
	m_venusTransformComponent.SetRotation(0.0f);
	m_venusTransformComponent.SetScale({ 1.0f, 1.0f });
}

}