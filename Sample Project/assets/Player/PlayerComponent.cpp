#include "PlayerComponent.h"
#include "PlatformComponent.h"
#include "PlayerHealthComponent.h"
#include "YouDiedMessageComponent.h"
#include "FlowerComponent.h"
#include "GameManagerComponent.h"

#include "Log.h"
#include "PhysicsAPI.h"



namespace Game
{

static constexpr float s_shotCooldownTolerance(0.04f);

PlayerComponentScriptComponentFormGenerator PlayerComponentScriptComponentFormGenerator::s_generator;

PlayerComponent::PlayerComponent(const DCore::ConstructorArgs<PlayerComponent>& args)
	:
	m_movementVelocity(args.MovementVelocity),
	m_dashCooldown(args.DashCooldown),
	m_dashVelocity(args.DashVelocity),
	m_jumpInitialVelocity(args.JumpInitialVelocity),
	m_gravityScale(args.GravityScale),
	m_shotVelocity(args.ShotVelocity),
	m_shotMaterial(args.ShotMaterial),
	m_shotAnimationStateMachine(args.ShotAnimationStateMachine),
	m_shotCooldown(args.ShotCooldown),
	m_blinkPeriod(args.BlinkPeriod),
	m_blinkTime(args.BlinkTime),
	m_stateTickFunction(nullptr),
	m_shootSound(args.ShootSound),
	m_dashSound(args.DashSound),
	m_jumpSound(args.JumpSound),
	m_landSound(args.LandSound),
	m_hitSound1(args.HitSound1),
	m_hitSound2(args.HitSound2),
	m_hitSound3(args.HitSound3),
	m_shotHitSound(args.ShotHitSound),
	m_deathSound(args.DeathSound)
{}

PlayerComponent::~PlayerComponent()
{
	m_runtime->RemoveFromOnOverlapBegin(m_overlapBeginRegistrationIndex, m_hurtBoxColliderId);
	m_shotMaterial.Unload();
	m_shotAnimationStateMachine.Unload();
}

void* PlayerComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_movementVelocity:
		return &m_movementVelocity;
	case a_dashCooldown:
		return &m_dashCooldown;
	case a_dashVelocity:
		return &m_dashVelocity;
	case a_jumpInitialVelocity:
		return &m_jumpInitialVelocity;
	case a_gravityScale:
		return &m_gravityScale;
	case a_shotVelocity:
		return &m_shotVelocity;
	case a_shotMaterial:
		return &m_shotMaterial;
	case a_shotAnimationStateMachine:
		return &m_shotAnimationStateMachine;
	case a_shotCooldown:
		return &m_shotCooldown;
	case a_blinkPeriod:
		return &m_blinkPeriod;
	case a_blinkTime:
		return &m_blinkTime;
	case a_shootSound:
		return &m_shootSound;
	case a_dashSound:
		return &m_dashSound;
	case a_jumpSound:
		return &m_jumpSound;
	case a_landSound:
		return &m_landSound;
	case a_hitSound1:
		return &m_hitSound1;
	case a_hitSound2:
		return &m_hitSound2;
	case a_hitSound3:
		return &m_hitSound3;
	case a_shotHitSound:
		return &m_shotHitSound;
	case a_deathSound:
		return &m_deathSound;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void PlayerComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId) 
	{
	case a_movementVelocity:
		m_movementVelocity = *static_cast<float*>(newValue);
		m_movementVelocity = std::max(m_movementVelocity, 0.0f);
		return;
	case a_dashCooldown:
		m_dashCooldown = *static_cast<float*>(newValue);
		return;
	case a_dashVelocity:
		m_dashVelocity = *static_cast<float*>(newValue);
		return;
	case a_jumpInitialVelocity:
		m_jumpInitialVelocity = *static_cast<float*>(newValue);
		return;
	case a_gravityScale:
		m_gravityScale = std::max(0.0f, *static_cast<float*>(newValue));
		return;
	case a_shotVelocity:
		m_shotVelocity = std::max(0.0f, *static_cast<float*>(newValue));
		return;
	case a_shotMaterial:
		m_shotMaterial = *static_cast<DCore::DSpriteMaterial*>(newValue);
		return;
	case a_shotAnimationStateMachine:
		m_shotAnimationStateMachine = *static_cast<DCore::DAnimationStateMachine*>(newValue);
		return;
	case a_shotCooldown:
		m_shotCooldown = *static_cast<float*>(newValue);
		return;
	case a_blinkPeriod:
		m_blinkPeriod = *static_cast<float*>(newValue);
		return;
	case a_blinkTime:
		m_blinkTime = *static_cast<float*>(newValue);
		return;
	case a_shootSound:
		m_shootSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_dashSound:
		m_dashSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_jumpSound:
		m_jumpSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_landSound:
		m_landSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_hitSound1:
		m_hitSound1 = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_hitSound2:
		m_hitSound2 = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_hitSound3:
		m_hitSound3 = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_shotHitSound:
		m_shotHitSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_deathSound:
		m_deathSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	default:
		return;
	}
}

void PlayerComponent::Start()
{
	DASSERT_K(m_shootSound.Setup());
	DASSERT_K(m_dashSound.Setup());
	DASSERT_K(m_jumpSound.Setup());
	DASSERT_K(m_landSound.Setup());
	DASSERT_K(m_hitSound1.Setup());
	DASSERT_K(m_hitSound2.Setup());
	DASSERT_K(m_hitSound3.Setup());
	DASSERT_K(m_deathSound.Setup());
	GetShootPoints();
	InstantiateShots();
	m_currentState = PlayerState::Movement;
	m_context.IsDashing = false;
	m_context.IsDucking = false;
	m_context.ApplyGravity = true;
	m_context.CanMove = true;
	m_context.CanChangeScaleWithVelocity = true;
	m_context.ToEndDash = false;
	m_context.ToEndDuckBegin = false;
	m_context.AirDash = false;
	m_context.ReleasedJumpKeyWhileJumping = false;
	m_context.ResetVelocityOnUpdate = true;
	m_context.IsInvulnerable = false;
	m_context.MovementType = MovementType::UserInput;
	m_currentBlinkTime = 0.0f;
	m_currentBlinkPeriod = 0.0f;
	m_currentDashCooldown = 0.0f;
	m_transformComponent = m_entityRef.GetComponents<DCore::TransformComponent>();
	m_boxColliderComponent = m_entityRef.GetComponents<DCore::BoxColliderComponent>();
	m_gravity = 9.8f;
	m_grounded = true;
	m_velocity = {0.0f, 0.0f};
	m_accelerationX = 0.0f;
	m_currentShotCooldown = 0.0f;
	m_entityRef.IterateOnChildren
	(
		[&](DCore::EntityRef entity) -> bool
		{
			DCore::DString name;
			entity.GetName(name);
			if (name == "Hurt Box")
			{
				m_hurtBoxColliderComponent = entity.GetComponents<DCore::BoxColliderComponent>();
				m_hurtBoxColliderId = m_hurtBoxColliderComponent.GetBodyId();
				m_overlapBeginRegistrationIndex = m_runtime->RegisterToOnOverlapBegin(GenerateComponentRefConstructorArgs(), m_hurtBoxColliderId);
				return false;
			}
			if (!entity.HaveComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent>())
			{
				return false;
			}
			auto [spriteComponent, asmComponent] = entity.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent>();
			m_spriteComponent = spriteComponent;
			m_asmComponent = asmComponent;
			return true;
		}
	);
	const DCore::ComponentIdType playerHealthComponentId(DCore::ComponentId::GetId<PlayerHealthComponent>());
	IterateOnEntitiesWithComponents(
		&playerHealthComponentId, 1,
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			m_playerHealthComponent = component;
			return true;
		});
	GetAnimationParameterIndexes();
	IntoIntroState();
}

void PlayerComponent::Update(float deltaTime)
{
	if (m_currentDashCooldown > 0.0f)
	{
		m_currentDashCooldown -= deltaTime;
	}
	if (m_currentShotCooldown > 0.0f)
	{
		m_currentShotCooldown -= deltaTime;
	}
	if (m_currentBlinkTime > 0.0f)
	{
		if (m_currentBlinkPeriod <= 0.0f)
		{
			m_currentBlinkPeriod = m_blinkPeriod;
			const float currentAlphaValue(m_spriteComponent.GetTintColor().a);
			m_spriteComponent.SetTintColor({1.0f, 1.0f, 1.0f, currentAlphaValue == 1.0f ? 0.25f : 1.0f});
		}
		m_currentBlinkTime -= deltaTime;
		m_currentBlinkPeriod -= deltaTime;
		if (m_currentBlinkTime <= 0.0f)
		{
			m_context.IsInvulnerable = false;
			m_spriteComponent.SetTintColor({1.0f, 1.0f, 1.0f, 1.0f});
			const DCore::DVec3 translation(m_transformComponent.GetTranslation());
			const DCore::DVec2 boxColliderOffset(m_hurtBoxColliderComponent.GetOffset());
			const DCore::DVec2 boxColliderTranslation(translation.x + boxColliderOffset.x, translation.y + boxColliderOffset.y);
			DCore::EntityRef damageArea;
			DCore::Physics::OverlapResult result;
			result.Entities = &damageArea;
			if (OverlapBox(
						m_transformComponent.GetRotation(), 
						m_hurtBoxColliderComponent.GetSizes(), 
						boxColliderTranslation, 
						DCore::Physics::GetLayersMask(m_hurtBoxColliderComponent.GetSelfPhysicsLayer()), 
						DCore::Physics::GetLayersMask(m_hurtBoxColliderComponent.GetCollideWithPhysicsLayers()), 
						&result, 1))
			{
				Hit(damageArea);
			}
		}
	}
}

void PlayerComponent::LateUpdate(float deltaTime)
{
//const DCore::DVec3 currentTranslation(m_transformComponent.GetTranslation());
//const DCore::DVec2 boxColliderSizes(m_boxColliderComponent.GetSizes());
//const DCore::DVec2 boxColliderOffset(m_boxColliderComponent.GetOffset());
//const DCore::DVec2 boxColliderTranslation(currentTranslation.x + boxColliderOffset.x, currentTranslation.y + boxColliderOffset.y);
//DrawDebugBox(boxColliderTranslation + m_velocity * 1.0f/60.0f, 0.0f, boxColliderSizes, {0.0f, 0.0f, 1.0f, 1.0f});
}

void PlayerComponent::PhysicsUpdate(float physicsDeltaTime)
{
	const DCore::DVec2 userInput(GetDirectionalInput());
	//
	// Movement
	if (m_context.ResetVelocityOnUpdate)
	{
		m_velocity.x = 0.0f;
	}
	if (m_context.ApplyGravity)
	{
		m_velocity.y -= m_gravity * m_gravityScale * physicsDeltaTime;
	}
	if (m_currentState == PlayerState::Movement && m_context.IsDashing)
	{
		const float scaleX(m_transformComponent.GetScale().x);
		m_velocity.x = m_dashVelocity * scaleX;
	}
	else if (m_context.CanMove)
	{
		switch (m_context.MovementType)
		{
		case MovementType::UserInput:
			m_velocity.x = m_movementVelocity * userInput.x;
			break;
		case MovementType::Acceleration:
			m_velocity.x += m_accelerationX * physicsDeltaTime;
			break;
		}
	}
	if (m_context.CanChangeScaleWithVelocity)
	{
		const DCore::DVec2 currentScale(m_transformComponent.GetScale());
		DCore::DVec2 targetScale(currentScale);
		if (m_velocity.x > 0.0f)
		{
			targetScale.x = 1.0f;
		}
		else if (m_velocity.x < 0.0f)
		{
			targetScale.x = -1.0f;
		}
		m_transformComponent.SetScale(targetScale);
	}
	const DCore::DVec3 currentTranslation(m_transformComponent.GetTranslation());
	const DCore::DVec2 boxColliderSizes(m_boxColliderComponent.GetSizes());
	const DCore::DVec2 boxColliderOffset(m_boxColliderComponent.GetOffset());
	const DCore::DVec2 boxColliderTranslation(currentTranslation.x + boxColliderOffset.x, currentTranslation.y + boxColliderOffset.y);
	const float deltaX(m_velocity.x * physicsDeltaTime);
	const float deltaY(m_velocity.y * physicsDeltaTime);
	constexpr float safetyDistanceY(0.2f);
	rayCastResultType castResult;
	// Y
	if (const float maxDistance{std::max(0.3f, glm::abs(deltaY)) + safetyDistanceY}; RayCastBox(0.0f, boxColliderSizes, boxColliderTranslation + DCore::DVec2{0.0f, deltaY > 0.0f ? -safetyDistanceY : safetyDistanceY}, {0.0f, deltaY > 0.0f ? 1.0f : -1.0f}, maxDistance, DCore::Physics::GetLayersMask(DCore::Physics::PhysicsLayer::Layer2), &castResult) && castResult.Normal.y > 0.0f)
	{
		m_velocity.y = 0.0f;
		if (!m_grounded)
		{
			m_landSound.Start();
		}
		m_grounded = true;
		const float newTranslationY(castResult.Point.y + boxColliderSizes.y / 2.0f - boxColliderOffset.y);
		m_transformComponent.SetTranslation({m_transformComponent.GetTranslation().x, newTranslationY, 0.0f});
		if (castResult.Entity.HaveComponents<PlatformComponent>())
		{
			m_platformComponent = castResult.Entity.GetComponent(DCore::ComponentId::GetId<PlatformComponent>());
			PlatformComponent* platform(static_cast<PlatformComponent*>(m_platformComponent.GetRawComponent()));
			platform->BeginPlayerCollision();
		}
	}
	else
	{
		m_grounded = false;
		m_transformComponent.AddTranslation({0.0f, deltaY, 0.0f});
		if (m_platformComponent.IsValid())
		{
			PlatformComponent* platform(static_cast<PlatformComponent*>(m_platformComponent.GetRawComponent()));
			platform->EndPlayerCollision();
			m_platformComponent.Invalidate();
		}
	}
	// X
	if (m_velocity.x != 0.0f && RayCastBox(0.0f, boxColliderSizes, boxColliderTranslation, glm::normalize(DCore::DVec2(m_velocity.x, 0.0f)), glm::abs(deltaX), DCore::Physics::GetLayersMask(DCore::Physics::PhysicsLayer::Layer2), &castResult))
	{
		constexpr float nearInterval(0.1f);
		bool positiveNormal(castResult.Normal.x <= 1.0f + nearInterval && castResult.Normal.x >= 1.0f - nearInterval);
		bool negativeNormal(castResult.Normal.x <= -1.0f + nearInterval && castResult.Normal.x >= -1.0f - nearInterval);
		if (positiveNormal || negativeNormal)
		{
			m_velocity.x = 0.0f;
			const float newTranslationOffset(boxColliderSizes.x / 2.0f + physicsDeltaTime / 1.5f);
			const float newTranslationX(castResult.Point.x + (positiveNormal ? newTranslationOffset : -newTranslationOffset) - boxColliderOffset.x);
			m_transformComponent.SetTranslation({newTranslationX, m_transformComponent.GetTranslation().y, 0.0f});
		}
	}
	else
	{
		m_transformComponent.AddTranslation({deltaX, 0.0f, 0.0f});
	}
}

void PlayerComponent::AnimationUpdate(float animationDeltaTime)
{
	if (m_stateTickFunction != nullptr)
	{
		(this->*m_stateTickFunction)();
	}
}

void PlayerComponent::OnOverlapBegin(DCore::EntityRef entity)
{
	if (m_context.IsInvulnerable)
	{
		return;
	}
	Hit(entity);
}

void PlayerComponent::ShootStraight()
{
	if (m_currentShotCooldown > s_shotCooldownTolerance)
	{
		return;
	}
	m_shootSound.Start();
	m_currentShotCooldown = m_shotCooldown;
	for (DCore::ComponentRef<DCore::Component> shotComponentRef : m_shotComponents)
	{
		PlayerShotComponent* shotComponent(static_cast<PlayerShotComponent*>(shotComponentRef.GetRawComponent()));
		if (!shotComponent->IsFree())
		{
			continue;
		}
		if (m_transformComponent.GetScale().x > 0.0f)
		{
			shotComponent->Fire({m_shotVelocity, 0.0f}, m_straightShootPoint.GetWorldTranslation(), 0.0f, false);
			return;
		}
		shotComponent->Fire({-m_shotVelocity, 0.0f}, m_straightShootPoint.GetWorldTranslation(), 0.0f, true);	
		return;	
	}
}

void PlayerComponent::ShootUp()
{
	if (m_currentShotCooldown > s_shotCooldownTolerance)
	{
		return;
	}
	m_shootSound.Start();
	m_currentShotCooldown = m_shotCooldown;
	for (DCore::ComponentRef<DCore::Component> shotComponentRef : m_shotComponents)
	{
		PlayerShotComponent* shotComponent(static_cast<PlayerShotComponent*>(shotComponentRef.GetRawComponent()));
		if (!shotComponent->IsFree())
		{
			continue;
		}
		shotComponent->Fire({0.0f, m_shotVelocity}, m_upShootPoint.GetWorldTranslation(), 90.0f, false);
		return;	
	}
}

void PlayerComponent::ShootDown()
{
	if (m_currentShotCooldown > s_shotCooldownTolerance)
	{
		return;
	}
	m_shootSound.Start();
	m_currentShotCooldown = m_shotCooldown;
	for (DCore::ComponentRef<DCore::Component> shotComponentRef : m_shotComponents)
	{
		PlayerShotComponent* shotComponent(static_cast<PlayerShotComponent*>(shotComponentRef.GetRawComponent()));
		if (!shotComponent->IsFree())
		{
			continue;
		}
		shotComponent->Fire({0.0f, -m_shotVelocity}, m_downShootPoint.GetWorldTranslation(), -90.0f, false);
		return;	
	}
}

void PlayerComponent::ShootDiagonalUp()
{
	if (m_currentShotCooldown > s_shotCooldownTolerance)
	{
		return;
	}
	m_shootSound.Start();
	m_currentShotCooldown = m_shotCooldown;
	const float diagonalVelocity(m_shotVelocity / glm::sqrt(2));
	for (DCore::ComponentRef<DCore::Component> shotComponentRef : m_shotComponents)
	{
		PlayerShotComponent* shotComponent(static_cast<PlayerShotComponent*>(shotComponentRef.GetRawComponent()));
		if (!shotComponent->IsFree())
		{
			continue;
		}
		if (m_transformComponent.GetScale().x > 0.0f)
		{
			shotComponent->Fire({diagonalVelocity, diagonalVelocity}, m_diagonalUpShootPoint.GetWorldTranslation(), 45.0f, false);
			return;
		}
		shotComponent->Fire({-diagonalVelocity, diagonalVelocity}, m_diagonalUpShootPoint.GetWorldTranslation(), -45.0f, true);
		return;	
	}
}

void PlayerComponent::ShootDiagonalDown()
{
	if (m_currentShotCooldown > s_shotCooldownTolerance)
	{
		return;
	}
	m_shootSound.Start();
	m_currentShotCooldown = m_shotCooldown;
	const float diagonalVelocity(m_shotVelocity / glm::sqrt(2));
	for (DCore::ComponentRef<DCore::Component> shotComponentRef : m_shotComponents)
	{
		PlayerShotComponent* shotComponent(static_cast<PlayerShotComponent*>(shotComponentRef.GetRawComponent()));
		if (!shotComponent->IsFree())
		{
			continue;
		}
		if (m_transformComponent.GetScale().x > 0.0f)
		{
			shotComponent->Fire({diagonalVelocity, -diagonalVelocity}, m_diagonalDownShootPoint.GetWorldTranslation(), -45.0f, false);
			return;
		}
		shotComponent->Fire({-diagonalVelocity, -diagonalVelocity}, m_diagonalDownShootPoint.GetWorldTranslation(), 45.0f, true);
		return;	
	}
}

void PlayerComponent::ShootDuck()
{
	if (m_currentShotCooldown > s_shotCooldownTolerance)
	{
		return;
	}
	m_shootSound.Start();
	m_currentShotCooldown = m_shotCooldown;
	for (DCore::ComponentRef<DCore::Component> shotComponentRef : m_shotComponents)
	{
		PlayerShotComponent* shotComponent(static_cast<PlayerShotComponent*>(shotComponentRef.GetRawComponent()));
		if (!shotComponent->IsFree())
		{
			continue;
		}
		if (m_transformComponent.GetScale().x > 0.0f)
		{
			shotComponent->Fire({m_shotVelocity, 0.0f}, m_duckShootPoint.GetWorldTranslation(), 0.0f, false);
			return;
		}
		shotComponent->Fire({-m_shotVelocity, 0.0f}, m_duckShootPoint.GetWorldTranslation(), 0.0f, true);
		return;	
	}

}

void PlayerComponent::ShootInAir()
{
	const DCore::DVec2 directionalInput(GetDirectionalInput());
	if (directionalInput.x == 0.0f)
	{
		if (directionalInput.y == 0.0f)
		{
			ShootStraight();
			return;
		}
		if (directionalInput.y > 0.0f)
		{
			ShootUp();
			return;
		}
		ShootDown();
		return;
	}
	if (directionalInput.y > 0.0f)
	{
		ShootDiagonalUp();
		return;
	}
	if (directionalInput.y == 0.0f)
	{
		ShootStraight();
		return;
	}
	ShootDiagonalDown();
}

void PlayerComponent::FlowerDefeated()
{
	m_context.IsInvulnerable = true;
}

void PlayerComponent::IntroBeginEnd()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introBeginAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introLoopAnimationParameter, DCore::LogicParameter{ true });
}

void PlayerComponent::IntroLoopEnd()
{
	if (m_context.CurrentNumberOfIntroLoops++ >= m_context.NumberOfIntroLoops)
	{
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introLoopAnimationParameter, DCore::LogicParameter{ false });
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introEndAnimationParameter, DCore::LogicParameter{ true });
	}
}

void PlayerComponent::IntroEndEnd()
{
	OutOfIntroState();
	IntoMovementState();
}

void PlayerComponent::GetAnimationParameterIndexes()
{
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Movement", m_movementAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Run", m_runAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Dash", m_dashAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Idle", m_idleAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Duck Begin", m_duckBeginAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Duck Loop", m_duckLoopAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Jump", m_jumpAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Shoot Jumping", m_shootJumpingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Hit Begin", m_hitBeginAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Hit Loop", m_hitLoopAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Hit End", m_hitEndAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim", m_aimAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Straight", m_aimStraightAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Up", m_aimUpAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Down", m_aimDownAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Diagonal Up", m_aimDiagonalUpAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Diagonal Down", m_aimDiagonalDownAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Shoot", m_aimShootAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Shoot Straight", m_aimShootStraightAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Shoot Up", m_aimShootUpAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Shoot Down", m_aimShootDownAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Shoot Diagonal Up", m_aimShootDiagonalUpAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Aim Shoot Diagonal Down", m_aimShootDiagonalDownAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Free Shoot", m_freeShootAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Free Shoot R Diag Up", m_freeShootRunningDiagonalUpAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Free Shoot R Straight", m_freeShootRunningStraightAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Free Shoot Straight", m_freeShootStraightAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Free Shoot Up", m_freeShootUpAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Free Shoot Down", m_freeShootDownAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Free Shoot Duck", m_freeShootDuckAnimationParameter);
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Dead", m_deadAnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro", m_introAnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro Begin", m_introBeginAnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro Loop", m_introLoopAnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro End", m_introEndAnimationParameter));
}

void PlayerComponent::IntoMovementState()
{
	const DCore::DVec2 directionInput(GetDirectionalInput());
	const bool dashInput(GetDashInput());
	m_currentState = PlayerState::Movement;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_movementAnimationParameter, DCore::LogicParameter{true});
	m_stateTickFunction = &PlayerComponent::TickMovementState;
	m_context.ResetVelocityOnUpdate = true;
	m_context.MovementType = MovementType::UserInput;
	TickMovementState();
}	

void PlayerComponent::TickMovementState()
{
	const DCore::DVec2 directionalInput(GetDirectionalInput());
	const bool dashInput(GetDashInput());
	const bool aimInput(GetAimInput());
	const bool shootInput(GetShootInput());
	const bool jumpInput(GetJumpInput());
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_runAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_dashAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_idleAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckBeginAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckLoopAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_jumpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_shootJumpingAnimationParameter, DCore::LogicParameter{false});
	if (m_grounded)
	{
		m_context.AirDash = false;
		m_context.ReleasedJumpKeyWhileJumping = false;
	}
	if (!m_grounded && !m_context.IsDashing)
	{
		m_context.IsDucking = false;
		m_context.ApplyGravity = true;
		m_context.CanMove = true;
		m_context.CanChangeScaleWithVelocity = true;
		m_context.ToEndDuckBegin = false;	
		if (shootInput)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_shootJumpingAnimationParameter, DCore::LogicParameter{true});
		}
		else
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_jumpAnimationParameter, DCore::LogicParameter{true});
		}
		if (!jumpInput && !m_context.ReleasedJumpKeyWhileJumping && m_velocity.y > 0.0f)
		{
			m_context.ReleasedJumpKeyWhileJumping = true;
			m_velocity.y *= 0.65f;
		}
	}
	if (m_context.IsDucking)
	{
		m_context.IsDashing = false;
		m_context.ApplyGravity = true;
		m_context.CanMove = false;
		m_context.CanChangeScaleWithVelocity = false;
		m_context.ToEndDash = false;
		m_context.AirDash = false;
		DirectionalInputScaleControl(directionalInput);
		if (m_context.ToEndDuckBegin)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckLoopAnimationParameter, DCore::LogicParameter{true});
		}
		else
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckBeginAnimationParameter, DCore::LogicParameter{true});
		}
		if (directionalInput.y >= 0.0f)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckLoopAnimationParameter, DCore::LogicParameter{false});
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckBeginAnimationParameter, DCore::LogicParameter{false});
			m_context.IsDucking = false;
			m_context.ToEndDuckBegin = false;
		}
	}
	auto transitions = [&]() -> void
	{
		if (dashInput && m_currentDashCooldown <= 0.0f && !m_context.AirDash)
		{
			m_context.IsDashing = true;
			m_context.IsDucking = false;
			m_context.ApplyGravity = false;
			m_context.CanMove = true;
			m_context.CanChangeScaleWithVelocity = false;
			m_context.ToEndDash = false;
			m_context.ToEndDuckBegin = false;
			m_context.AirDash = false;
			m_velocity.y = 0.0f;
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_dashAnimationParameter, DCore::LogicParameter{true});
			m_dashSound.Start();
			if (!m_grounded)
			{
				m_context.AirDash = true;
			}
			return;
		}
		if (jumpInput && m_grounded)
		{
			m_velocity.y = m_jumpInitialVelocity;
			m_context.IsDashing = false;
			m_context.IsDucking = false;
			m_context.ApplyGravity = true;
			m_context.CanMove = true;
			m_context.CanChangeScaleWithVelocity = true;
			m_context.ToEndDash = false;
			m_context.ToEndDuckBegin = false;
			m_context.AirDash = false;
			m_jumpSound.Start();
			return;
		}
		if (!m_context.IsDucking && m_grounded && aimInput)
		{
			OutOfMovementState();
			if (shootInput)
			{
				IntoAimShootState();
			}
			else
			{
				IntoAimState();
			}
			return;
		}
		if (m_grounded && shootInput)
		{
			OutOfMovementState();
			IntoFreeShootState();
			return;
		}
		if (!m_context.IsDucking && m_grounded && directionalInput.y < 0.0f)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckBeginAnimationParameter, DCore::LogicParameter{true});
			m_context.IsDashing = false;
			m_context.IsDucking = true;
			m_context.ApplyGravity = true;
			m_context.CanMove = false;
			m_context.CanChangeScaleWithVelocity = false;
			m_context.ToEndDash = false;
			m_context.ToEndDuckBegin = false;
			m_context.AirDash = false;
			return;
		}
		if (m_context.IsDucking || !m_grounded)
		{
			return;
		}
		m_context.IsDashing = false;
		m_context.IsDucking = false;
		m_context.ApplyGravity = true;
		m_context.CanMove = true;
		m_context.CanChangeScaleWithVelocity = true;
		m_context.ToEndDash = false;
		m_context.ToEndDuckBegin = false;
		m_context.AirDash = false;
		if (directionalInput.x == 0.0f)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_idleAnimationParameter, DCore::LogicParameter{true});
			return;
		}
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_runAnimationParameter, DCore::LogicParameter{true});
	};
	if (!m_context.IsDashing)
	{
		transitions();
		return;
	}
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_dashAnimationParameter, DCore::LogicParameter{true});
	if (m_context.ToEndDash)
	{
		m_currentDashCooldown = m_dashCooldown;
		m_context.IsDashing = false;
		transitions();
	}
}

void PlayerComponent::OutOfMovementState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_movementAnimationParameter, DCore::LogicParameter{false});			
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_idleAnimationParameter, DCore::LogicParameter{false});			
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_runAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_dashAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckBeginAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_duckLoopAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_jumpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_shootJumpingAnimationParameter, DCore::LogicParameter{false});
}

void PlayerComponent::IntoAimState()
{
	m_currentState = PlayerState::Aim;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimAnimationParameter, DCore::LogicParameter{true});
	m_stateTickFunction = &PlayerComponent::TickAimState;
	m_context.IsDashing = false;
	m_context.IsDucking = false;
	m_context.ApplyGravity = true;
	m_context.CanMove = false;
	m_context.CanChangeScaleWithVelocity = false;
	m_context.ToEndDash = false;
	m_context.ToEndDuckBegin = false;
	m_context.AirDash = false;
	m_context.ResetVelocityOnUpdate = true;
	m_context.MovementType = MovementType::UserInput;
	TickAimState();
}

void PlayerComponent::TickAimState()
{
	const DCore::DVec2 directionalInput(GetDirectionalInput());
	const bool dashInput(GetDashInput());
	const bool aimInput(GetAimInput());
	const bool shootInput(GetShootInput());
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDownAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDiagonalUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDiagonalDownAnimationParameter, DCore::LogicParameter{false});
	if ((directionalInput.x == 0.0f || directionalInput.x != 0.0f) && directionalInput.y == 0.0f)
	{
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimStraightAnimationParameter, DCore::LogicParameter{true});
	}
	else
	{
		if (directionalInput.x != 0.0f)
		{
			if (directionalInput.y > 0.0f)
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDiagonalUpAnimationParameter, DCore::LogicParameter{true});
			}
			else
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDiagonalDownAnimationParameter, DCore::LogicParameter{true});
			}
		}
		else
		{
			if (directionalInput.y > 0.0f)
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimUpAnimationParameter, DCore::LogicParameter{true});
			}
			else
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDownAnimationParameter, DCore::LogicParameter{true});
			}
		}
	}	
	DirectionalInputScaleControl(directionalInput);
	if (!m_grounded || (m_grounded && GetJumpInput()))
	{
		OutOfAimState();
		IntoMovementState();
		return;
	}
	if (dashInput && m_currentDashCooldown <= 0.0f)
	{
		OutOfAimState();
		IntoMovementState();
		return;
	}
	if (!aimInput)
	{
		OutOfAimState();
		if (shootInput)
		{
			IntoFreeShootState();
		}
		else
		{
			IntoMovementState();
		}
		return;
	}
	if (shootInput)
	{
		OutOfAimState();
		IntoAimShootState();
		return;
	}
}

void PlayerComponent::OutOfAimState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDownAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDiagonalUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimDiagonalDownAnimationParameter, DCore::LogicParameter{false});
}

void PlayerComponent::IntoAimShootState()
{
	m_currentState = PlayerState::AimShoot;
	m_stateTickFunction = &PlayerComponent::TickAimShootState;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootAnimationParameter, DCore::LogicParameter{true});
	m_context.IsDashing = false;
	m_context.IsDucking = false;
	m_context.ApplyGravity = true;
	m_context.CanMove = false;
	m_context.CanChangeScaleWithVelocity = false;
	m_context.ToEndDash = false;
	m_context.ToEndDuckBegin = false;
	m_context.AirDash = false;
	m_context.ResetVelocityOnUpdate = true;
	m_context.MovementType = MovementType::UserInput;
	TickAimShootState();
}

void PlayerComponent::TickAimShootState()
{
	const DCore::DVec2 directionalInput(GetDirectionalInput());
	const bool dashInput(GetDashInput());
	const bool aimInput(GetAimInput());
	const bool shootInput(GetShootInput());
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDownAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDiagonalUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDiagonalDownAnimationParameter, DCore::LogicParameter{false});
	if ((directionalInput.x == 0.0f || directionalInput.x != 0.0f) && directionalInput.y == 0.0f)
	{
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootStraightAnimationParameter, DCore::LogicParameter{true});
	}
	else
	{
		if (directionalInput.x != 0.0f)
		{
			if (directionalInput.y > 0.0f)
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDiagonalUpAnimationParameter, DCore::LogicParameter{true});
			}
			else
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDiagonalDownAnimationParameter, DCore::LogicParameter{true});
			}
		}
		else
		{
			if (directionalInput.y > 0.0f)
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootUpAnimationParameter, DCore::LogicParameter{true});
			}
			else
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDownAnimationParameter, DCore::LogicParameter{true});
			}
		}
	}
	DirectionalInputScaleControl(directionalInput);
	if (!m_grounded || (m_grounded && GetJumpInput()))
	{
		OutOfAimShootState();
		IntoMovementState();
		return;
	}
	if (dashInput && m_currentDashCooldown <= 0.0f)
	{
		OutOfAimShootState();
		IntoMovementState();
		return;
	}
	if (!shootInput)
	{
		OutOfAimShootState();
		if (aimInput)
		{
			IntoAimState();
		}
		else
		{
			IntoMovementState();
		}
		return;
	}
	if (!aimInput)
	{
		OutOfAimShootState();
		IntoFreeShootState();
		return;
	}
}

void PlayerComponent::OutOfAimShootState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDownAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDiagonalUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_aimShootDiagonalDownAnimationParameter, DCore::LogicParameter{false});
}

void PlayerComponent::IntoFreeShootState()
{
	m_currentState = PlayerState::FreeShoot;
	m_stateTickFunction = &PlayerComponent::TickFreeShootState;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootAnimationParameter, DCore::LogicParameter{true});
	m_context.IsDashing = false;
	m_context.ApplyGravity = true;
	m_context.CanMove = true;
	m_context.CanChangeScaleWithVelocity = true;
	m_context.ToEndDash = false;
	m_context.ToEndDuckBegin = false;
	m_context.AirDash = false;
	m_context.ResetVelocityOnUpdate = true;
	m_context.MovementType = MovementType::UserInput;
	TickFreeShootState();
}

void PlayerComponent::TickFreeShootState()
{
	const DCore::DVec2 directionalInput(GetDirectionalInput());
	const bool dashInput(GetDashInput());
	const bool aimInput(GetAimInput());
	const bool shootInput(GetShootInput());
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootRunningDiagonalUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootRunningStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootDownAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootDuckAnimationParameter, DCore::LogicParameter{false});
	if (directionalInput.y >= 0.0f)
	{
		m_context.IsDucking = false;
		m_context.CanMove = true;
		m_context.CanChangeScaleWithVelocity = true;
		m_context.ToEndDuckBegin = false;
	}
	if (m_context.IsDucking)
	{
		DirectionalInputScaleControl(directionalInput);
	}
	if (directionalInput.x == 0.0f)
	{
		if (directionalInput.y == 0.0f)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootStraightAnimationParameter, DCore::LogicParameter{true});
		}
		else if (directionalInput.y > 0.0f)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootUpAnimationParameter, DCore::LogicParameter{true});
		}
		else
		{
			m_context.IsDucking = true;
			m_context.CanMove = false;
			m_context.CanChangeScaleWithVelocity = false;
			m_context.ToEndDuckBegin = true;
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootDuckAnimationParameter, DCore::LogicParameter{true});
		}
	}
	else 
	{
		if (directionalInput.y > 0.0f)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootRunningDiagonalUpAnimationParameter, DCore::LogicParameter{true});
		}
		else if (directionalInput.y == 0.0f)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootRunningStraightAnimationParameter, DCore::LogicParameter{true});
		}
		else
		{
			m_context.IsDucking = true;
			m_context.CanMove = false;
			m_context.CanChangeScaleWithVelocity = false;
			m_context.ToEndDuckBegin = true;
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootDuckAnimationParameter, DCore::LogicParameter{true});
		}
	}
	if (!m_grounded || (m_grounded && GetJumpInput()))
	{
		OutOfFreeShootState();
		IntoMovementState();
		return;
	}
	if ((dashInput && m_currentDashCooldown <= 0.0f) || (!shootInput && !aimInput))
	{
		OutOfFreeShootState();
		IntoMovementState();
		return;
	}
	if (aimInput)
	{
		m_context.IsDucking = false;
		OutOfFreeShootState();
		if (shootInput)
		{
			IntoAimShootState();
			return;
		}
		IntoAimState();
		return;
	}
}

void PlayerComponent::OutOfFreeShootState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootRunningDiagonalUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootRunningStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootStraightAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootUpAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootDownAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_freeShootDuckAnimationParameter, DCore::LogicParameter{false});
}

void PlayerComponent::IntoHitState()
{
	OutOfMovementState();
	OutOfAimState();
	OutOfAimShootState();
	OutOfFreeShootState();
	m_stateTickFunction = &PlayerComponent::TickHitState;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitBeginAnimationParameter, DCore::LogicParameter{true});
	m_context.IsDashing = false;
	m_context.IsDucking = false;
	m_context.ApplyGravity = true;
	m_context.CanMove = true;
	m_context.CanChangeScaleWithVelocity = false;
	m_context.ToEndDash = false;
	m_context.ToEndDuckBegin = false;
	m_context.AirDash = false;
	m_context.ReleasedJumpKeyWhileJumping = false;
	m_context.ResetVelocityOnUpdate = false;
	m_context.IsInvulnerable = true;
	m_context.ToEndHitBegin = false;
	m_context.ToEndHitLoop = false;
	m_context.ToEndHitEnd = false;
	m_context.MovementType = MovementType::Acceleration;
	TickHitState();
}

void PlayerComponent::TickHitState()
{
	constexpr float stoppingVelocity(0.5f);
	if (glm::abs(m_velocity.x) <= stoppingVelocity)
	{
		m_accelerationX = 0.0f;
	}
	DirectionalInputScaleControl(GetDirectionalInput());
	if (m_context.ToEndHitBegin)
	{
		m_context.ToEndHitBegin = false;
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitBeginAnimationParameter, DCore::LogicParameter{false});
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitLoopAnimationParameter, DCore::LogicParameter{true});
	}
	if (m_context.ToEndHitLoop)
	{
		m_context.ToEndHitLoop = false;
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitLoopAnimationParameter, DCore::LogicParameter{false});
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitEndAnimationParameter, DCore::LogicParameter{true});
	}
	if (m_context.ToEndHitEnd)
	{
		m_currentBlinkTime = m_blinkTime;
		m_currentBlinkPeriod = m_blinkPeriod;
		m_context.ToEndHitEnd = false;
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitEndAnimationParameter, DCore::LogicParameter{false});
		const DCore::DVec2 directionalInput(GetDirectionalInput());
		const bool jumpInput(GetJumpInput());
		const bool dashInput(GetDashInput());
		const bool aimInput(GetAimInput());
		const bool shootInput(GetShootInput());	
		OutOfHitState();
		if (jumpInput || dashInput)
		{
			IntoMovementState();
			return;
		}
		if (aimInput)
		{
			if (!shootInput)
			{
				IntoAimState();
				return;
			}
			IntoAimShootState();
			return;
		}
		if (shootInput)
		{
			IntoFreeShootState();
			return;
		}
		IntoMovementState();
		return;
	}
}

void PlayerComponent::OutOfHitState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitBeginAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitLoopAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_hitEndAnimationParameter, DCore::LogicParameter{false});
}

void PlayerComponent::IntoDeadState()
{
	OutOfMovementState();
	OutOfAimState();
	OutOfAimShootState();
	OutOfFreeShootState();
	OutOfHitState();
	m_currentState = PlayerState::Dead;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_deadAnimationParameter, DCore::LogicParameter{ true });
	m_context.ApplyGravity = false;
	m_context.CanMove = false;
	m_context.CanChangeScaleWithVelocity = false;
	m_context.IsInvulnerable = true;
	m_context.MovementType = MovementType::UserInput;
	m_velocity.x = 0.0f;
	m_velocity.y = 1.0f;
	m_stateTickFunction = nullptr;
}

void PlayerComponent::IntoIntroState()
{
	m_context.IsDashing = false;
	m_context.IsDucking = false;
	m_context.ApplyGravity = true;
	m_context.CanMove = false;
	m_context.NumberOfIntroLoops = 3;
	m_context.CurrentNumberOfIntroLoops = 0;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introAnimationParameter, DCore::LogicParameter{ true });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introBeginAnimationParameter, DCore::LogicParameter{ true });
}

void PlayerComponent::OutOfIntroState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introBeginAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introLoopAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introEndAnimationParameter, DCore::LogicParameter{ false });
}

void PlayerComponent::GetShootPoints()
{
	m_entityRef.IterateOnChildren
	(
		[&](DCore::EntityRef entity) -> bool
		{
			DCore::DString entityName;
			entity.GetName(entityName);
			if (entityName == "Straight Shoot Point")
			{
				m_straightShootPoint = entity;
			}
			else if (entityName == "Up Shoot Point")
			{
				m_upShootPoint = entity;
			}
			else if (entityName == "Down Shoot Point")
			{
				m_downShootPoint = entity;
			}
			else if (entityName == "Diagonal Up Shoot Point")
			{
				m_diagonalUpShootPoint = entity;
			}
			else if (entityName == "Diagonal Down Shoot Point")
			{
				m_diagonalDownShootPoint = entity;
			}
			else if (entityName == "Duck Shoot Point")
			{
				m_duckShootPoint = entity;
			}
			return false;
		}
	);
	DASSERT_E(m_straightShootPoint.IsValid());
}

void PlayerComponent::InstantiateShots()
{
	for (size_t i(0); i < numberOfShots; i++)
	{
		const DCore::ConstructorArgs<DCore::BoxColliderComponent> boxColliderArgs(
			DCore::DBodyType::Dynamic,
			false,
			false,
			0.0f,
			true,
			true,
			{0.3f, 0.0f},
			{0.5f, 0.1f},
			DCore::PhysicsMaterialRef(),
			DCore::DPhysicsLayer::Layer6,
			DCore::DPhysicsLayer::Layer5);
		const DCore::ConstructorArgs<DCore::SpriteComponent> spriteArgs(
			false,
			false,
			0,
			{6, 2},
			7,
			80,
			{1.0f, 1.0f, 1.0f, 1.0f},
			DCore::AssetManager::Get().GetSpriteMaterial(m_shotMaterial.GetUUID()),
			true);
		DCore::AnimationStateMachine shotAnimationStateMachine(m_shotAnimationStateMachine.GetInternalRef().Data()->GetAsset());
		DCore::DAnimationStateMachine shotAnimationStateMachineRef(DCore::AssetManager::Get().AddAnimationStateMachine(std::move(shotAnimationStateMachine)));
		const DCore::ConstructorArgs<DCore::AnimationStateMachineComponent> asmArgs{shotAnimationStateMachineRef}; 
		DCore::EntityRef shotEntity(
			CreateEntity<PlayerShotComponent, DCore::BoxColliderComponent, DCore::SpriteComponent, DCore::AnimationStateMachineComponent>(
				"Shot", 
				std::make_tuple(DCore::ConstructorArgs<PlayerShotComponent>(m_shotHitSound)),
				std::make_tuple(boxColliderArgs),
				std::make_tuple(spriteArgs),
				std::make_tuple(asmArgs)));
		auto [transform, boxCollider, animationStateMachineComponent] = shotEntity.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent, DCore::AnimationStateMachineComponent>();
		transform.SetTranslation({2.0f, 9.0f, 0.0f});
		boxCollider.SetBodyType(DCore::DBodyType::Dynamic);
		m_shotComponents.PushBack(shotEntity.GetComponent(DCore::ComponentId::GetId<PlayerShotComponent>()));
		animationStateMachineComponent.AttachToEntity(shotEntity);
	}
}

void PlayerComponent::Hit(DCore::EntityRef entity)
{
	if (m_context.IsInvulnerable)
	{
		return;
	}
	PlayerHealthComponent* playerHealthComponent(static_cast<PlayerHealthComponent*>(m_playerHealthComponent.GetRawComponent()));
	const DCore::DUInt currentHealth(playerHealthComponent->GetCurrentHealth());
	if (currentHealth == 3)
	{
		m_hitSound1.Start();
	}
	else if (currentHealth == 2)
	{
		m_hitSound2.Start();
	}
	else
	{
		m_hitSound3.Start();
	}
	if (playerHealthComponent->GetCurrentHealth() > 0)
	{
		playerHealthComponent->DecreaseHealth();
		if (playerHealthComponent->GetCurrentHealth() == 0)
		{
			m_deathSound.Start();
			IntoDeadState();
			const DCore::ComponentIdType youDiedMessageComponentId(DCore::ComponentId::GetId<YouDiedMessageComponent>());
			const DCore::ComponentIdType flowerComponentId(DCore::ComponentId::GetId<FlowerComponent>());
			const DCore::ComponentIdType gameManagerComponentId(DCore::ComponentId::GetId<GameManagerComponent>());
			IterateOnEntitiesWithComponents(
				&youDiedMessageComponentId, 1,
				[&](DCore::Entity entity, DCore::ComponentRef<DCore::Component> component) -> bool
				{
					YouDiedMessageComponent* youDiedMessageComponent(static_cast<YouDiedMessageComponent*>(component.GetRawComponent()));
					youDiedMessageComponent->Display();
					return true;
				});
			IterateOnEntitiesWithComponents(
				&flowerComponentId, 1,
				[&](DCore::Entity entity, DCore::ComponentRef<DCore::Component> component) -> bool
				{
					FlowerComponent* flowerComponent(static_cast<FlowerComponent*>(component.GetRawComponent()));
					flowerComponent->PlayerDied();
					return true;
				});
			IterateOnEntitiesInAllScenesWithComponents(
				&gameManagerComponentId, 1,
				[&](DCore::Entity entity, DCore::ComponentRef<DCore::Component> component) -> bool
				{
					GameManagerComponent* gameManagerComponent(static_cast<GameManagerComponent*>(component.GetRawComponent()));
					gameManagerComponent->PlayerDied();
					return true;
				});
			return;
		}
	}
	IntoHitState();
	constexpr float hitVelocityX(2.0f);
	constexpr float hitVelocityY(14.0f);
	constexpr float accelerationX(2.0f);
	m_velocity.y = hitVelocityY;
	m_context.HitDirection = entity.GetComponents<DCore::TransformComponent>().GetTranslation().x - m_transformComponent.GetTranslation().x >= 0.0f ? 1 : -1;
	if (m_context.HitDirection == 1)
	{
		m_velocity.x = -hitVelocityX;
		m_accelerationX = accelerationX;
		return;
	}
	m_velocity.x = hitVelocityX;
	m_accelerationX = -accelerationX;
}

}
