#pragma once

#include "DommusCore.h"

#include "BoxColliderParams.h"

#include <cstddef>
#include <random>



namespace Game
{
	class FlowerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::FlowerComponent>
{
	ConstructorArgs()
	{}
	
	float MinimumAttackCooldownTime;
	float MaximumAttackCooldownOffset;
	DCore::DUInt Health;
	DCore::DSoundEventInstance IntroSound;
	DCore::DSoundEventInstance FaceAttackBeginSound;
	DCore::DSoundEventInstance FaceAttackLoopSound;
	DCore::DSoundEventInstance FaceAttackEndSound;
	DCore::DSoundEventInstance GunAttackBeginSound;
	DCore::DSoundEventInstance GunAttackLoopSound;
	DCore::DSoundEventInstance GunAttackEndSound;
	DCore::DSoundEventInstance CreateSound;
	DCore::DSoundEventInstance KnockoutSound;
};
#pragma pack(pop)

namespace Game
{

class FlowerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_minimumAttackCooldownTime{0};
	static constexpr size_t a_maximumAttackCooldownOffset{1};
	static constexpr size_t a_health{2};
	static constexpr size_t a_introSound{3};
	static constexpr size_t a_faceAttackBeginSound{4};
	static constexpr size_t a_faceAttackLoopSound{5};
	static constexpr size_t a_faceAttackEndSound{6};
	static constexpr size_t a_gunAttackBeginSound{7};
	static constexpr size_t a_gunAttackLoopSound{8};
	static constexpr size_t a_gunAttackEndSound{9};
	static constexpr size_t a_createSound{10};
	static constexpr size_t a_knockoutSound{11};
public:
	FlowerComponent(const DCore::ConstructorArgs<FlowerComponent>&);
	~FlowerComponent();
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
	virtual void OnAnimationEvent(size_t eventId) override;
	virtual void OnOverlapBegin(DCore::EntityRef) override;
public:
	void PlayerDied();
private:
	static constexpr size_t numberOfAttacks{3};
	static constexpr size_t numberOfUpFaceAttackBeginPrepareBoxColliderParams{ 4 };
	static constexpr size_t numberOfUpFaceAttackBeginAttackBoxColliderParams{ 5 };
	static constexpr size_t numberOfUpFaceAttackEndAttackBoxColliderParams{ 5 };
	static constexpr size_t numberOfDownFaceAttackBeginPrepareBoxColliderParams{ 4 };
	static constexpr size_t numberOfDownFaceAttackBeginAttackBoxColliderParams{ 5 };
	static constexpr size_t numberOfDownFaceAttackEndAttackBoxColliderParams{ 5 };
private:
	enum class StateContextType
	{
		Idle,
		Throw,
		FaceAttack,
		GunAttack,
		Intro
	};

	typedef
	struct IdleStateContext
	{
		float TimeInIdleState;
		float CurrentTimeInIdleState;
	} IdleStateContext;

	typedef
	struct ThrowAttackContext
	{
		size_t NumberOfAttacks;
		size_t CurrentAttackCount;
		bool IsFirstBoomerang;
		size_t NumberOfCreatingAnimationLoops;
		size_t CurrentNumberOfCreatingAnimationLoops;
		size_t NumberOfThrowingAnimationLoops;
		size_t CurrentNumberOfThrowingAnimationLoops;
	} ThrowAttackContext;

	typedef
	struct FaceAttackContext
	{
		size_t NumberOfPreparationLoops;
		size_t CurrentNumberOfPreparationLoops;
		size_t NumberOfAttackLoops;
		size_t CurrentNumberOfAttackLoops;
		bool AttackUp;
	} FaceAttackContext;

	typedef
	struct GunAttackContext
	{
		size_t NumberOfAttackingLoops;
		size_t CurrentNumberOfAttackingLoops;
	} GunAttackContext;

	typedef
	struct IntroStateContext
	{
		size_t NumberOfIntro1;
		size_t CurrentNumberOfIntro1;
		size_t NumberOfIntroLoops;
		size_t CurrentNumberOfIntroLoops;
	} IntroStateContext;

	typedef
	union UStateContext
	{
		FlowerComponent::IdleStateContext IdleStateContext;
		FlowerComponent::ThrowAttackContext ThrowAttackContext;
		FlowerComponent::FaceAttackContext FaceAttackContext;
		FlowerComponent::GunAttackContext GunAttackContext;
		FlowerComponent::IntroStateContext IntroStateContext;
	} UStateContext;

	typedef
	struct StateContext
	{
		StateContextType Type;
		UStateContext StateUnion;
	} StateContext;
private:
	// Editor
	float m_minimumAttackCooldownTime;
	float m_maximumAttackCooldownOffset;
	DCore::DUInt m_health;
	DCore::DSoundEventInstance m_introSound;
	DCore::DSoundEventInstance m_faceAttackBeginSound;
	DCore::DSoundEventInstance m_faceAttackLoopSound;
	DCore::DSoundEventInstance m_faceAttackEndSound;
	DCore::DSoundEventInstance m_gunAttackBeginSound;
	DCore::DSoundEventInstance m_gunAttackLoopSound;
	DCore::DSoundEventInstance m_gunAttackEndSound;
	DCore::DSoundEventInstance m_createSound;
	DCore::DSoundEventInstance m_knockoutSound;
	// Runtime
	size_t m_projectilesAnimationParameter;
	size_t m_createBeginAnimationParameter;
	size_t m_creatingAnimationParameter;
	size_t m_throwBeginAnimationParameter;
	size_t m_throwingAnimationParameter;
	size_t m_createResetAnimationParameter;
	size_t m_createEndAnimationParameter;
	size_t m_upFaceAttackAnimationParameter;
	size_t m_upFaceAttackBeginPrepareAnimationParameter;
	size_t m_upFaceAttackPreparingAnimationParameter;
	size_t m_upFaceAttackBeginAttackAnimationParameter;
	size_t m_upFaceAttackAttackingAnimationParameter;
	size_t m_upFaceAttackEndAttackAnimationParameter;
	size_t m_downFaceAttackAnimationParameter;
	size_t m_downFaceAttackBeginPrepareAnimationParameter;
	size_t m_downFaceAttackPreparingAnimationParameter;
	size_t m_downFaceAttackBeginAttackAnimationParameter;
	size_t m_downFaceAttackAttackingAnimationParameter;
	size_t m_downFaceAttackEndAttackAnimationParameter;
	size_t m_gunAttackAnimationParameter;
	size_t m_gunAttackBeginAttackAnimationParameter;
	size_t m_gunAttackAttackingAnimationParameter;
	size_t m_gunAttackEndAttackAnimationParameter;
	size_t m_deathAnimationParameter;
	size_t m_introAnimationParameter;
	size_t m_intro1AnimationParameter;
	size_t m_intro2AnimationParameter;
	size_t m_introLoopAnimationParameter;
	size_t m_introEndAnimationParameter;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	DCore::ComponentRef<DCore::Component> m_acornManagerComponent;
	DCore::ComponentRef<DCore::Component> m_boomerangManagerComponent;
	DCore::ComponentRef<DCore::Component> m_seedEnemiesLauncherComponent;
	DCore::ComponentRef<DCore::TransformComponent> m_hurtBoxTransformComponent;
	DCore::ComponentRef<DCore::BoxColliderComponent> m_hurtBoxComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_seedMissilesSpriteComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_seedMissilesAsmComponent;
	size_t m_seedMissilesFireAnimationParameter;
	DCore::DBodyId m_hurtBoxBodyId;
	size_t m_shotOverlapBeginRegistrationIndex;
	DCore::EntityRef m_attackHitBox;
	StateContext m_stateContext;
	size_t m_lastExecutedAttackId;
	std::discrete_distribution<> m_attackDistribution;
	DCore::Array<BoxColliderParams, numberOfUpFaceAttackBeginPrepareBoxColliderParams> m_upFaceAttackBeginPrepareBoxColliderParams;
	DCore::Array<BoxColliderParams, numberOfUpFaceAttackBeginAttackBoxColliderParams> m_upFaceAttackBeginAttackBoxColliderParams;
	BoxColliderParams m_upFaceAttackAttackingBoxColliderParams;
	DCore::Array<BoxColliderParams, numberOfUpFaceAttackEndAttackBoxColliderParams> m_upFaceAttackEndAttackBoxColliderParams;
	DCore::Array<BoxColliderParams, numberOfDownFaceAttackBeginPrepareBoxColliderParams> m_downFaceAttackBeginPrepareBoxColliderParams;
	DCore::Array<BoxColliderParams, numberOfDownFaceAttackBeginAttackBoxColliderParams> m_downFaceAttackBeginAttackBoxColliderParams;
	BoxColliderParams m_downFaceAttackAttackingBoxColliderParams;
	DCore::Array<BoxColliderParams, numberOfDownFaceAttackEndAttackBoxColliderParams> m_downFaceAttackEndAttackBoxColliderParams;
	DCore::DUInt m_currentHealth;
	bool m_playerDied;
private:
	void GetAnimationParametersIndexes();
	void GetFaceAttackHitBoxes();
	void SetupHurtBox();
	void IntoIdleState();
	void IntoThrowAttack();
	void OutOfThrowAttack();
	void LaunchAcorns();
	void LaunchBoomerang();
	void IntoFaceAttack();
	void OutOfFaceAttack();
	void IntoGunAttack();
	void OutOfGunAttack();
	void IntoDeathState();
	void IntoIntroState();
	void OutOfIntroState();
};

class FlowerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~FlowerComponentScriptComponentFormGenerator() = default;
private:
	FlowerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"FlowerComponent",
			DCore::ComponentId::GetId<FlowerComponent>(),
			sizeof(FlowerComponent),
			sizeof(DCore::ConstructorArgs<FlowerComponent>),
			{{DCore::AttributeName("Minimum Attack Cooldown Time"), DCore::AttributeType::Float, FlowerComponent::a_minimumAttackCooldownTime},
			{DCore::AttributeName("Maximum Attack Cooldown Offset"), DCore::AttributeType::Float, FlowerComponent::a_maximumAttackCooldownOffset},
			{DCore::AttributeName("Health"), DCore::AttributeType::UInteger, FlowerComponent::a_health},
			{DCore::AttributeName("Intro Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_introSound},
			{DCore::AttributeName("Face Attack Begin Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_faceAttackBeginSound},
			{DCore::AttributeName("Face Attack Loop Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_faceAttackLoopSound},
			{DCore::AttributeName("Face Attack End Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_faceAttackEndSound},
			{DCore::AttributeName("Gun Attack Begin Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_gunAttackBeginSound},
			{DCore::AttributeName("Gun Attack Loop Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_gunAttackLoopSound},
			{DCore::AttributeName("Gun Attack End Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_gunAttackEndSound},
			{DCore::AttributeName("Create Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_createSound},
			{DCore::AttributeName("Knockout Sound"), DCore::AttributeType::SoundEventInstance, FlowerComponent::a_knockoutSound}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) FlowerComponent(*static_cast<const DCore::ConstructorArgs<FlowerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<FlowerComponent*>(componentAddress)->~FlowerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<FlowerComponent> m_defaultArgs;
private:
	static FlowerComponentScriptComponentFormGenerator s_generator;
};

}
