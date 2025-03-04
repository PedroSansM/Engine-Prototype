#pragma once

#include "PlayerShotComponent.h"

#include "DommusCore.h"



namespace Game
{
	class PlayerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::PlayerComponent>
{
	ConstructorArgs()
		:
		MovementVelocity(6.5f),
		DashCooldown(0.1f),
		DashVelocity(10.0f),
		JumpInitialVelocity(10.0f),
		GravityScale(1.0f),
		ShotVelocity(10.0f),
		ShotCooldown(0.15f),
		BlinkPeriod(0.5f),
		BlinkTime(3.0f)
	{}

	float MovementVelocity;
	float DashCooldown;
	float DashVelocity;
	float JumpInitialVelocity;
	float GravityScale;
	float ShotVelocity;
	DCore::DSpriteMaterial ShotMaterial;
	DCore::DAnimationStateMachine ShotAnimationStateMachine;
	float ShotCooldown;
	float BlinkPeriod;
	float BlinkTime;
	DCore::DSoundEventInstance ShootSound;
	DCore::DSoundEventInstance DashSound;
	DCore::DSoundEventInstance JumpSound;
	DCore::DSoundEventInstance LandSound;
	DCore::DSoundEventInstance HitSound1;
	DCore::DSoundEventInstance HitSound2;
	DCore::DSoundEventInstance HitSound3;
	DCore::DSoundEventInstance ShotHitSound;
	DCore::DSoundEventInstance DeathSound;
};
#pragma pack(pop)

namespace Game
{

enum class ShotDirection
{
	None,
	Straight,
	Up,
	Down,
	DiagonalUp,
	DiagonalDown
};

class PlayerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_movementVelocity{0};
	static constexpr size_t a_dashCooldown{1};
	static constexpr size_t a_dashVelocity{2};
	static constexpr size_t a_jumpInitialVelocity{3};
	static constexpr size_t a_gravityScale{4};
	static constexpr size_t a_shotVelocity{5};
	static constexpr size_t a_shotMaterial{6};
	static constexpr size_t a_shotAnimationStateMachine{7};
	static constexpr size_t a_shotCooldown{8};
	static constexpr size_t a_blinkPeriod{9};
	static constexpr size_t a_blinkTime{10};
	static constexpr size_t a_shootSound{11};
	static constexpr size_t a_dashSound{12};
	static constexpr size_t a_jumpSound{13};
	static constexpr size_t a_landSound{14};
	static constexpr size_t a_hitSound1{15};
	static constexpr size_t a_hitSound2{16};
	static constexpr size_t a_hitSound3{17};
	static constexpr size_t a_shotHitSound{18};
	static constexpr size_t a_deathSound{19};
public:
	static constexpr size_t numberOfShots{100};
	static constexpr size_t numberOfPlatforms{3};
public:
	PlayerComponent(const DCore::ConstructorArgs<PlayerComponent>&);
	~PlayerComponent();
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate(float deltaTime) override;
	virtual void PhysicsUpdate(float physicsDeltaTime) override;
	virtual void AnimationUpdate(float animationDeltaTime) override;
	virtual void OnOverlapBegin(DCore::EntityRef) override;
public:
	void ShootStraight();
	void ShootUp();
	void ShootDown();
	void ShootDiagonalUp();
	void ShootDiagonalDown();
	void ShootDuck();
	void ShootInAir();
	void FlowerDefeated();
	void IntroBeginEnd();
	void IntroLoopEnd();
	void IntroEndEnd();
public:
	void OnDashEnd()
	{
		m_context.ToEndDash = true;
	}
	
	void OnDuckBeginEnd()
	{
		m_context.ToEndDuckBegin = true;
	}

	void EndDashBegin()
	{
		m_context.ToEndHitBegin = true;
	}
	 
	void EndDashLoop()
	{
		m_context.ToEndHitLoop = true;
	}

	void EndDashEnd()
	{
		m_context.ToEndHitEnd = true;
	}

	void ToEndHitBegin()
	{
		m_context.ToEndHitBegin = true;
	}

	void ToEndHitLoop()
	{
		m_context.ToEndHitLoop = true;
	}

	void ToEndHitEnd()
	{
		m_context.ToEndHitEnd = true;
	}
private:
	enum class PlayerState : uint64_t
	{
		Movement,
		Aim,
		AimShoot,
		FreeShoot,
		Dead,
		NumberOfStates
	};

	enum class MovementType
	{
		UserInput,
		Acceleration
	};
private:
	typedef
	struct Context
	{
		bool IsDashing;
		bool IsDucking;
		bool ApplyGravity;
		bool CanMove;
		bool CanChangeScaleWithVelocity;
		bool ToEndDash;
		bool ToEndDuckBegin;
		bool AirDash;
		bool ReleasedJumpKeyWhileJumping;
		bool ResetVelocityOnUpdate;
		bool IsInvulnerable;
		bool ToEndHitBegin;
		bool ToEndHitLoop;
		bool ToEndHitEnd;
		int HitDirection;
		size_t NumberOfIntroLoops;
		size_t CurrentNumberOfIntroLoops;
		PlayerComponent::MovementType MovementType;
	} Context;
private:
	// Editor
	float m_movementVelocity;
	float m_dashCooldown;
	float m_dashVelocity;
	float m_jumpInitialVelocity;
	float m_gravityScale;
	float m_shotVelocity;
	float m_shotCooldown;
	float m_blinkPeriod;
	float m_blinkTime;
	DCore::DSpriteMaterial m_shotMaterial;
	DCore::DAnimationStateMachine m_shotAnimationStateMachine;
	DCore::DSoundEventInstance m_shootSound;
	DCore::DSoundEventInstance m_dashSound;
	DCore::DSoundEventInstance m_jumpSound;
	DCore::DSoundEventInstance m_landSound;
	DCore::DSoundEventInstance m_hitSound1;
	DCore::DSoundEventInstance m_hitSound2;
	DCore::DSoundEventInstance m_hitSound3;
	DCore::DSoundEventInstance m_shotHitSound;
	DCore::DSoundEventInstance m_deathSound;
	// State
	PlayerState m_currentState;
	void (PlayerComponent::*m_stateTickFunction)();
	Context m_context;
	float m_currentBlinkTime;
	float m_currentBlinkPeriod;
	float m_currentDashCooldown;
	size_t m_movementAnimationParameter;
	size_t m_runAnimationParameter;
	size_t m_dashAnimationParameter;
	size_t m_idleAnimationParameter;
	size_t m_duckBeginAnimationParameter;
	size_t m_duckLoopAnimationParameter;
	size_t m_jumpAnimationParameter;
	size_t m_shootJumpingAnimationParameter;
	size_t m_hitBeginAnimationParameter;
	size_t m_hitLoopAnimationParameter;
	size_t m_hitEndAnimationParameter;
	size_t m_aimAnimationParameter;
	size_t m_aimStraightAnimationParameter;
	size_t m_aimUpAnimationParameter;
	size_t m_aimDownAnimationParameter;
	size_t m_aimDiagonalUpAnimationParameter;
	size_t m_aimDiagonalDownAnimationParameter;
	size_t m_aimShootAnimationParameter;
	size_t m_aimShootStraightAnimationParameter;
	size_t m_aimShootUpAnimationParameter;
	size_t m_aimShootDownAnimationParameter;
	size_t m_aimShootDiagonalUpAnimationParameter;
	size_t m_aimShootDiagonalDownAnimationParameter;
	size_t m_freeShootAnimationParameter;
	size_t m_freeShootRunningDiagonalUpAnimationParameter;
	size_t m_freeShootRunningStraightAnimationParameter;
	size_t m_freeShootStraightAnimationParameter;
	size_t m_freeShootUpAnimationParameter;
	size_t m_freeShootDownAnimationParameter;
	size_t m_freeShootDuckAnimationParameter;
	size_t m_deadAnimationParameter;
	size_t m_introAnimationParameter;
	size_t m_introBeginAnimationParameter;
	size_t m_introLoopAnimationParameter;
	size_t m_introEndAnimationParameter;
	// Physics
	float m_gravity;
	bool m_grounded;
	DCore::DVec2 m_velocity;
	float m_accelerationX;
	size_t m_overlapBeginRegistrationIndex;
	DCore::DBodyId m_hurtBoxColliderId;
	// Components
	DCore::ComponentRef<DCore::TransformComponent> m_transformComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_boxColliderComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_hurtBoxColliderComponent;
	// Shots
	DCore::Array<DCore::ComponentRef<DCore::Component>, numberOfShots> m_shotComponents;
	DCore::EntityRef m_straightShootPoint;
	DCore::EntityRef m_upShootPoint;
	DCore::EntityRef m_downShootPoint;
	DCore::EntityRef m_diagonalUpShootPoint;
	DCore::EntityRef m_diagonalDownShootPoint;
	DCore::EntityRef m_duckShootPoint;
	float m_currentShotCooldown;
	// Floating platforms
	DCore::ComponentRef<DCore::Component> m_platformComponent;
	// Player Health
	DCore::ComponentRef<DCore::Component> m_playerHealthComponent;
private:
	void GetAnimationParameterIndexes();
	void IntoMovementState();
	void TickMovementState();
	void OutOfMovementState();
	void IntoAimState();
	void TickAimState();
	void OutOfAimState();
	void IntoAimShootState();
	void TickAimShootState();
	void OutOfAimShootState();
	void IntoFreeShootState();
	void TickFreeShootState();
	void OutOfFreeShootState();
	void IntoHitState();
	void TickHitState();
	void OutOfHitState();
	void IntoDeadState();
	void IntoIntroState();
	void OutOfIntroState();
	void GetShootPoints();
	void InstantiateShots();
	void Hit(DCore::EntityRef);
private:
	DCore::DVec2 GetDirectionalInput() const
	{
		DCore::DVec2 userInput(0.0f, 0.0f);
		if (m_runtime->KeyPressed(DCore::DKey::ArrowRight))
		{
			userInput.x = 1.0f;
		}
		else if (m_runtime->KeyPressed(DCore::DKey::ArrowLeft))
		{
			userInput.x = -1.0f;
		}
		if (m_runtime->KeyPressed(DCore::DKey::ArrowUp))
		{
			userInput.y = 1.0f;
		}
		else if (m_runtime->KeyPressed(DCore::DKey::ArrowDown))
		{
			userInput.y = -1.0f;
		}
		return userInput;
	}

	bool GetDashInput() const
	{
		return m_runtime->KeyPressed(DCore::DKey::L_Shift);
	}

	bool GetAimInput() const
	{
		return m_runtime->KeyPressed(DCore::DKey::C);
	}

	bool GetShootInput() const
	{
		return m_runtime->KeyPressed(DCore::DKey::X);
	}

	bool GetJumpInput() const
	{
		return m_runtime->KeyPressed(DCore::DKey::Z);
	}

	void DirectionalInputScaleControl(const DCore::DVec2& directionalInput)
	{
		if (directionalInput.x == 0.0f)
		{
			return;
		}
		m_transformComponent.SetScale({directionalInput.x, 1.0f});
	}
};

class PlayerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~PlayerComponentScriptComponentFormGenerator() = default;
private:
	PlayerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"PlayerComponent",
			DCore::ComponentId::GetId<PlayerComponent>(),
			sizeof(PlayerComponent),
			sizeof(DCore::ConstructorArgs<PlayerComponent>),
			{{DCore::AttributeName("Movement Velocity"), DCore::AttributeType::Float, PlayerComponent::a_movementVelocity},
			{DCore::AttributeName("Dash Cooldown"), DCore::AttributeType::Float, PlayerComponent::a_dashCooldown},
			{DCore::AttributeName("Dash Velocity"), DCore::AttributeType::Float, PlayerComponent::a_dashVelocity},
			{DCore::AttributeName("Jump Initial Velocity"), DCore::AttributeType::Float, PlayerComponent::a_jumpInitialVelocity},
			{DCore::AttributeName("Gravity Scale"), DCore::AttributeType::Float, PlayerComponent::a_gravityScale},
			{DCore::AttributeName("Shot velocity"), DCore::AttributeType::Float, PlayerComponent::a_shotVelocity},
			{DCore::AttributeName("Shot Material"), DCore::AttributeType::SpriteMaterial, PlayerComponent::a_shotMaterial},
			{DCore::AttributeName("Shot Animation State Machine"), DCore::AttributeType::AnimationStateMachine, PlayerComponent::a_shotAnimationStateMachine},
			{DCore::AttributeName("Shot Cooldown"), DCore::AttributeType::Float, PlayerComponent::a_shotCooldown},
			{DCore::AttributeName("Blink Period"), DCore::AttributeType::Float, PlayerComponent::a_blinkPeriod},
			{DCore::AttributeName("Blink Time"), DCore::AttributeType::Float, PlayerComponent::a_blinkTime},
			{DCore::AttributeName("Shoot Sound"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_shootSound},
			{DCore::AttributeName("Dash Sound"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_dashSound},
			{DCore::AttributeName("Jump Sound"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_jumpSound},
			{DCore::AttributeName("Land Sound"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_landSound},
			{DCore::AttributeName("Hit Sound 1"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_hitSound1},
			{DCore::AttributeName("Hit Sound 2"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_hitSound2},
			{DCore::AttributeName("Hit Sound 3"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_hitSound3},
			{DCore::AttributeName("Shot Hit Sound"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_shotHitSound},
			{DCore::AttributeName("Death Sound"), DCore::AttributeType::SoundEventInstance, PlayerComponent::a_deathSound} },
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) PlayerComponent(*static_cast<const DCore::ConstructorArgs<PlayerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<PlayerComponent*>(componentAddress)->~PlayerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<PlayerComponent> m_defaultArgs;
private:
	static PlayerComponentScriptComponentFormGenerator s_generator;
};

}
