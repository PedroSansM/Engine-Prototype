#include "AcornComponent.h"
#include "PlayerComponent.h"

#include "Log.h"



namespace Game
{

AcornComponentScriptComponentFormGenerator AcornComponentScriptComponentFormGenerator::s_generator;

AcornComponent::AcornComponent(const DCore::ConstructorArgs<AcornComponent>& args)
	:
	m_initialVelocity(args.InitialVelocity),
	m_acceleration(args.Acceleration),
	m_lifeTime(args.LifeTime)
{}

void* AcornComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_initialVelocity:
		return &m_initialVelocity;
	case a_acceleration:
		return &m_acceleration;
	case a_lifeTime:
		return &m_lifeTime;
	default:
		return nullptr;
	}
	return nullptr;
}

void AcornComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_initialVelocity:
		m_initialVelocity = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_acceleration:
		m_acceleration = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_lifeTime:
		m_lifeTime = std::max(*static_cast<float*>(newValue), 1.0f);
		return;
	default:
		return;
	}
}

void AcornComponent::Start()
{
	DCore::ComponentIdType components[]{DCore::ComponentId::GetId<PlayerComponent>(), DCore::ComponentId::GetId<DCore::TransformComponent>()};
	IterateOnEntitiesWithComponents
	(
		components, 2,
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{	
			if (component.GetComponentId() == DCore::ComponentId::GetId<DCore::TransformComponent>())
			{
				m_playerTransformComponent = component;
				return true;
			}
			return false;
		}
	);
	auto [spriteComponent, asmComponent, boxColliderComponent, transformComponent] = m_entityRef.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent, DCore::BoxColliderComponent, DCore::TransformComponent>();
	m_transformComponent = transformComponent;
	m_spriteComponent = spriteComponent;
	m_asmComponent = asmComponent;
	m_boxColliderComponent = boxColliderComponent;
	m_initialPosition = m_transformComponent.GetTranslation();
	m_launched = false;
	m_spriteComponent.SetEnabled(false);
	m_boxColliderComponent.SetEnabled(false);
	GetAnimationParametersIndexes();
}

void AcornComponent::PhysicsUpdate(float physicsDeltaTime)
{
	if (!m_launched)
	{
		return;
	}
	m_currentLifeTime -= physicsDeltaTime;
	const DCore::DVec2 dv(m_direction * m_acceleration * physicsDeltaTime);
	m_boxColliderComponent.SetLinearVelocity(m_boxColliderComponent.GetLinearVelocity() + dv);
	if (m_currentLifeTime <= 0.0f)
	{
		m_spriteComponent.SetEnabled(false);
		m_boxColliderComponent.SetEnabled(false);
		m_boxColliderComponent.SetLinearVelocity({0.0f, 0.0f});
		m_transformComponent.SetTranslation(m_initialPosition);
		m_launched = false;
	}
}

void AcornComponent::Enable()
{
	m_spriteComponent.SetEnabled(true);
	m_boxColliderComponent.SetEnabled(true);
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_spinAnimationParameterIndex, DCore::LogicParameter{true});
}

void AcornComponent::Launch()
{
	m_currentLifeTime = m_lifeTime;
	const DCore::TransformComponent* playerTransformComponent(static_cast<DCore::TransformComponent*>(m_playerTransformComponent.GetRawComponent()));
	m_direction = glm::normalize(playerTransformComponent->GetTranslation() - m_transformComponent.GetTranslation());
	m_boxColliderComponent.SetLinearVelocity(m_direction * m_initialVelocity);
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_spinAnimationParameterIndex, DCore::LogicParameter{false});
	float acornRotation(glm::degrees(glm::atan(m_direction.y / m_direction.x)));
	DCore::DVec2 acornScale({1.0f, 1.0f});
	if (m_direction.x > 0.0f)
	{
		acornScale *= -1.0f;
	}
	m_transformComponent.SetRotation(acornRotation);
	m_transformComponent.SetScale(acornScale);
	m_launched = true;
}

void AcornComponent::GetAnimationParametersIndexes()
{
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Spin", m_spinAnimationParameterIndex);
}

}
