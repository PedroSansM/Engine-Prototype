#include "PlayerShotComponent.h"



namespace Game
{

PlayerShotComponentScriptComponentFormGenerator PlayerShotComponentScriptComponentFormGenerator::s_generator;

void PlayerShotComponent::Fire(const DCore::DVec2& velocity, const DCore::DVec2& initialPosition, float rotation, bool toFlip)
{
	m_transformComponent.SetTranslation({initialPosition.x, initialPosition.y, 0.0f});
	m_transformComponent.SetRotation(rotation);
	m_transformComponent.SetScale({toFlip ? -1.0f : 1.0f, 1.0f});
	m_boxColliderComponent.SetLinearVelocity(velocity);
	m_boxColliderComponent.SetEnabled(true);
	m_isFree = false;
	m_timeAlive = 4.0f;
}

void PlayerShotComponent::Kill()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_deadAnimationParameter, DCore::LogicParameter{ true });
	m_boxColliderComponent.SetEnabled(false);
	m_timeAlive = 0.0f;
	m_hitSound.Start();
}

}
