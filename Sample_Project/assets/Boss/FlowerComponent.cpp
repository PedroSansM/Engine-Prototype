#include "FlowerComponent.h"
#include "AcornManagerComponent.h"
#include "BoomerangManagerComponent.h"
#include "SeedEnemiesLauncherComponent.h"
#include "PlayerShotComponent.h"
#include "PlayerComponent.h"
#include "KnockoutComponent.h"
#include "GameManagerComponent.h"

#include "Log.h"

#include <random>



namespace Game
{

static std::mt19937 s_randomGenerator(std::random_device{}());

FlowerComponentScriptComponentFormGenerator FlowerComponentScriptComponentFormGenerator::s_generator;

FlowerComponent::FlowerComponent(const DCore::ConstructorArgs<FlowerComponent>& args)
	:
	m_minimumAttackCooldownTime(args.MinimumAttackCooldownTime),
	m_maximumAttackCooldownOffset(args.MaximumAttackCooldownOffset),
	m_health(args.Health),
	m_introSound(args.IntroSound),
	m_faceAttackBeginSound(args.FaceAttackBeginSound),
	m_faceAttackLoopSound(args.FaceAttackLoopSound),
	m_faceAttackEndSound(args.FaceAttackEndSound),
	m_gunAttackBeginSound(args.GunAttackBeginSound),
	m_gunAttackLoopSound(args.GunAttackLoopSound),
	m_gunAttackEndSound(args.GunAttackEndSound),
	m_createSound(args.CreateSound),
	m_knockoutSound(args.KnockoutSound),
	m_upFaceAttackBeginPrepareBoxColliderParams(
		{
			{{3.0f, 3.0f}, {4.764f, 2.694f}},
			{{3.0f, 3.0f}, {5.093f, 2.335f}},
			{{3.0f, 3.0f}, {5.871f, 2.096f}},
			{{3.0f, 3.0f}, {6.200f, 2.096f}} }),
	m_upFaceAttackBeginAttackBoxColliderParams(
		{ 
			{{3.0f, 3.0f}, {5.077f, 2.329f}},
			{{3.0f, 3.0f}, {3.452f, 2.735f}},
			{{4.0f, 4.0f}, {1.277f, 2.830f}},
			{{6.0f, 4.0f}, {0.034f, 2.830f}},
			{{10.0f, 4.0f}, {-1.497f, 2.830f}} }),
	m_upFaceAttackAttackingBoxColliderParams{ {12.0f, 5.0f}, {-3.227f, 2.415f} },
	m_upFaceAttackEndAttackBoxColliderParams(
		{ 
			{{10.0f, 4.0f}, {-1.497f, 2.830f}},
			{{6.0f, 4.0f}, {0.000f, 2.830f}},
			{{3.0f, 3.0f}, {3.328f, 2.830f}},
			{{3.0f, 3.0f}, {3.115f, 2.830f}},
			{{3.0f, 3.0f}, {3.622f, 2.830f}} }),
	m_downFaceAttackBeginPrepareBoxColliderParams(
		{
			{{3.0f, 3.0f}, {4.567f, 2.571f}},
			{{3.0f, 3.0f}, {5.183f, 1.140f}},
			{{3.0f, 3.0f}, {5.554f, 0.140f}},
			{{3.0f, 3.0f}, {5.554f, -0.118f}} }),
	m_downFaceAttackBeginAttackBoxColliderParams(
		{
			{{3.0f, 3.0f}, {5.208f, -0.414f}},
			{{3.0f, 3.0f}, {3.925f, -0.883f}},
			{{4.0f, 3.0f}, {2.223f, -1.451f}},
			{{6.0f, 3.0f}, {0.297f, -2.265f}},
			{{12.0f, 3.0f}, {-2.885f, -1.993f}} }),
	m_downFaceAttackAttackingBoxColliderParams{ {13.0f, 3.0f}, {-3.225f, -1.993f} },
	m_downFaceAttackEndAttackBoxColliderParams(
		{
			{{10.0f, 3.0f}, {-1.305f, -1.993f}},
			{{4.0f, 3.0f}, {2.618f, 1.707f}},
			{{3.0f, 3.0f}, {3.541f, 1.707f}},
			{{3.0f, 3.0f}, {4.419f, 2.275f}} })
{}

FlowerComponent::~FlowerComponent()
{
	m_runtime->RemoveFromOnOverlapBegin(m_shotOverlapBeginRegistrationIndex, m_hurtBoxBodyId);
}

void* FlowerComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_minimumAttackCooldownTime:
		return &m_minimumAttackCooldownTime;	
	case a_maximumAttackCooldownOffset:
		return &m_maximumAttackCooldownOffset;
	case a_health:
		return &m_health;
	case a_introSound:
		return &m_introSound;
	case a_faceAttackBeginSound:
		return &m_faceAttackBeginSound;
	case a_faceAttackLoopSound:
		return &m_faceAttackLoopSound;
	case a_faceAttackEndSound:
		return &m_faceAttackEndSound;
	case a_gunAttackBeginSound:
		return &m_gunAttackBeginSound;
	case a_gunAttackLoopSound:
		return &m_gunAttackLoopSound;
	case a_gunAttackEndSound:
		return &m_gunAttackEndSound;
	case a_createSound:
		return &m_createSound;
	case a_knockoutSound:
		return &m_knockoutSound;
	default:
		return nullptr;
	}
	return nullptr;
}

void FlowerComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId) 
	{
	case a_minimumAttackCooldownTime:
		m_minimumAttackCooldownTime = std::max(*static_cast<float*>(newValue), 1.0f);	
		return;
	case a_maximumAttackCooldownOffset:
		m_maximumAttackCooldownOffset = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_health:
		m_health = *static_cast<DCore::DUInt*>(newValue);
		return;
	case a_introSound:
		m_introSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;
	case a_faceAttackBeginSound:
		m_faceAttackBeginSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;	
	case a_faceAttackLoopSound:
		m_faceAttackLoopSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;	
	case a_faceAttackEndSound:
		m_faceAttackEndSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;		
	case a_gunAttackBeginSound:
		m_gunAttackBeginSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;			
	case a_gunAttackLoopSound:
		m_gunAttackLoopSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;			
	case a_gunAttackEndSound:
		m_gunAttackEndSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;			
	case a_createSound:
		m_createSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;			
	case a_knockoutSound:
		m_knockoutSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;		
	default:
		break;
	}
	DASSERT_E(false);
}

void FlowerComponent::Start()
{
	DASSERT_K(m_introSound.Setup());
	DASSERT_K(m_faceAttackBeginSound.Setup());
	DASSERT_K(m_faceAttackLoopSound.Setup());
	DASSERT_K(m_faceAttackEndSound.Setup());
	DASSERT_K(m_gunAttackBeginSound.Setup());
	DASSERT_K(m_gunAttackLoopSound.Setup());
	DASSERT_K(m_gunAttackEndSound.Setup());
	DASSERT_K(m_createSound.Setup());
	DASSERT_K(m_knockoutSound.Setup());
	m_playerDied = false;
	m_asmComponent = m_entityRef.GetComponents<DCore::AnimationStateMachineComponent>(); 
	const DCore::ComponentIdType acornManagerComponentId(DCore::ComponentId::GetId<AcornManagerComponent>());
	DCore::Array<double, numberOfAttacks> attackWeights;
	for (size_t i(0); i < numberOfAttacks; i++)
	{
		attackWeights.PushBack(1);
	}
	m_attackDistribution.param(decltype(m_attackDistribution)::param_type(&attackWeights[0], attackWeights.end()));
	m_lastExecutedAttackId = m_attackDistribution(s_randomGenerator);
	IterateOnEntitiesWithComponents
	(
		&acornManagerComponentId, 1,
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> acornManagerComponent) -> bool
		{
			m_acornManagerComponent = acornManagerComponent;
			return true;
		}
	);
	const DCore::ComponentIdType boomerangManagerComponentId(DCore::ComponentId::GetId<BoomerangManagerComponent>());
	IterateOnEntitiesWithComponents
	(
		&boomerangManagerComponentId, 1, 
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> boomerangManagerComponent) -> bool
		{
			m_boomerangManagerComponent = boomerangManagerComponent;
			return true;
		}
	);
	const DCore::ComponentIdType seedEnemiesLauncherComponentId(DCore::ComponentId::GetId<SeedEnemiesLauncherComponent>());
	IterateOnEntitiesWithComponents
	(
		&seedEnemiesLauncherComponentId, 1, 
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			m_seedEnemiesLauncherComponent = component;
			return true;
		}
	);
	m_entityRef.IterateOnChildren(
		[&](DCore::EntityRef child) -> bool
		{
			DCore::DString name;
			child.GetName(name);
			if (name == "Seed Missiles")
			{
				auto [spriteComponent, asmComponent] = child.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent>();
				m_seedMissilesSpriteComponent = spriteComponent;
				m_seedMissilesAsmComponent = asmComponent;
				DASSERT_E(m_seedMissilesSpriteComponent.IsValid() && m_seedMissilesAsmComponent.IsValid());
				m_seedMissilesSpriteComponent.SetEnabled(false);
				DASSERT_K(m_seedMissilesAsmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Fire", m_seedMissilesFireAnimationParameter));
				return true;
			}
			return false;
		});
	m_currentHealth = m_health;
	GetAnimationParametersIndexes();
	GetFaceAttackHitBoxes();
	SetupHurtBox();
	IntoIntroState();
}

void FlowerComponent::Update(float deltaTime)
{
	constexpr size_t throwAttackId(0);
	constexpr size_t faceAttackId(1);
	constexpr size_t gunAttackId(2);
	switch (m_stateContext.Type)
	{
	case StateContextType::Idle:
	{
		if (m_playerDied)
		{
			return;
		}
		if (m_stateContext.StateUnion.IdleStateContext.CurrentTimeInIdleState > 0.0f)
		{
			m_stateContext.StateUnion.IdleStateContext.CurrentTimeInIdleState -= deltaTime;
		}
		if (m_stateContext.StateUnion.IdleStateContext.CurrentTimeInIdleState <= 0.0f)
		{
			DCore::Array<double, numberOfAttacks> weights;
			for (size_t i(0); i < numberOfAttacks; i++)
			{
				if (i != m_lastExecutedAttackId)
				{
					weights.PushBack(1);
					continue;
				}
				weights.PushBack(0);
			}
			m_attackDistribution.param(decltype(m_attackDistribution)::param_type(&weights[0], weights.end()));
			const size_t attackToExecute(m_attackDistribution(s_randomGenerator));
			m_lastExecutedAttackId = attackToExecute;
			switch (attackToExecute)
			{
			case throwAttackId:
				IntoThrowAttack();
				break;
			case faceAttackId:
				IntoFaceAttack();
				break;
			case gunAttackId:
				IntoGunAttack();
				break;
			default:
				break;
			}
		}
		return;
	}
	default:
		return;
	}
}

void FlowerComponent::OnMetachannelEvent(size_t eventId)
{
	constexpr size_t createBeginEnd(0);
	constexpr size_t creatingEnd(1);
	constexpr size_t throwBeginEnd(2);
	constexpr size_t throwingEnd(3);
	constexpr size_t createResetEnd(4);
	constexpr size_t createEndEnd(5);
	constexpr size_t faceAttackBeginPrepareEnd(6);
	constexpr size_t faceAttackPreparingEnd(7);
	constexpr size_t faceAttackBeginAttackEnd(8);
	constexpr size_t faceAttackAttackingEnd(9);
	constexpr size_t faceAttackEndAttackEnd(10);
	constexpr size_t upFaceAttackBeginAttackFrame0(13);
	constexpr size_t upFaceAttackBeginAttackFrame1(14);
	constexpr size_t upFaceAttackBeginAttackFrame2(15);
	constexpr size_t upFaceAttackBeginAttackFrame3(16);
	constexpr size_t upFaceAttackBeginAttackFrame4(17);
	constexpr size_t upFaceAttackAttackingFrame(19);
	constexpr size_t upFaceAttackEndAttackFrame0(20);
	constexpr size_t upFaceAttackEndAttackFrame1(21);
	constexpr size_t upFaceAttackEndAttackFrame2(22);
	constexpr size_t upFaceAttackEndAttackFrame3(23);
	constexpr size_t upFaceAttackEndAttackFrame4(24);
	constexpr size_t upFaceAttackBeginPrepareFrame0(25);
	constexpr size_t upFaceAttackBeginPrepareFrame1(26);
	constexpr size_t upFaceAttackBeginPrepareFrame2(27);
	constexpr size_t upFaceAttackBeginPrepareFrame3(28);
	constexpr size_t downFaceAttackBeginPrepareFrame0(29);
	constexpr size_t downFaceAttackBeginPrepareFrame1(30);
	constexpr size_t downFaceAttackBeginPrepareFrame2(31);
	constexpr size_t downFaceAttackBeginPrepareFrame3(32);
	constexpr size_t downFaceAttackBeginAttackFrame0(33);
	constexpr size_t downFaceAttackBeginAttackFrame1(34);
	constexpr size_t downFaceAttackBeginAttackFrame2(35);
	constexpr size_t downFaceAttackBeginAttackFrame3(36);
	constexpr size_t downFaceAttackBeginAttackFrame4(37);
	constexpr size_t downFaceAttackAttackingFrame(38);
	constexpr size_t downFaceAttackEndAttackFrame0(39);
	constexpr size_t downFaceAttackEndAttackFrame1(40);
	constexpr size_t downFaceAttackEndAttackFrame2(41);
	constexpr size_t downFaceAttackEndAttackFrame3(42);
	constexpr size_t gunAttackBeginAttackEnd(43);
	constexpr size_t gunAttackAttackingEnd(44);
	constexpr size_t gunAttackEndAttackEnd(45);
	constexpr size_t idleBegin(46);
	constexpr size_t gunAttackBegin(47);
	constexpr size_t gunAttackAttackingBegin(48);
	constexpr size_t gunAttackAttackEndBegin(49);
	constexpr size_t intro1End(50);
	constexpr size_t intro2End(51);
	constexpr size_t introLoopEnd(52);
	constexpr size_t introEndEnd(53);
	constexpr size_t createBegin(54);
	switch (eventId)
	{
	case createBeginEnd:
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createBeginAnimationParameter, DCore::LogicParameter{false});
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_creatingAnimationParameter, DCore::LogicParameter{true});
		return;
	case creatingEnd:
		m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfCreatingAnimationLoops++;
		if (m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfCreatingAnimationLoops >= m_stateContext.StateUnion.ThrowAttackContext.NumberOfCreatingAnimationLoops)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_creatingAnimationParameter, DCore::LogicParameter{false});
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_throwBeginAnimationParameter, DCore::LogicParameter{true});
		}
		return;
	case throwBeginEnd:
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_throwBeginAnimationParameter, DCore::LogicParameter{false});
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_throwingAnimationParameter, DCore::LogicParameter{true});
		if (m_stateContext.StateUnion.ThrowAttackContext.IsFirstBoomerang)
		{
			if (m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount == 0 ||
				m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount == 2)
			{
				LaunchBoomerang();
			}
			else
			{
				LaunchAcorns();
			}
		}
		else
		{
			if (m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount == 0 ||
				m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount == 2)
			{
				LaunchAcorns();
			}
			else
			{
				LaunchBoomerang();
			}
		}
		m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount++;
		return;
	case throwingEnd:
		m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfThrowingAnimationLoops++;
		if (m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfThrowingAnimationLoops >= m_stateContext.StateUnion.ThrowAttackContext.NumberOfThrowingAnimationLoops)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_throwingAnimationParameter, DCore::LogicParameter{false});
			// To perform more throw attacks?
			if (m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount >= m_stateContext.StateUnion.ThrowAttackContext.NumberOfAttacks)
			{
				// Perform create end animation.
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createEndAnimationParameter, DCore::LogicParameter{true});
				return;
			}
			m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfCreatingAnimationLoops = 0;
			m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfThrowingAnimationLoops = 0;
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createResetAnimationParameter, DCore::LogicParameter{true});
		}
		return;
	case createResetEnd:
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createResetAnimationParameter, DCore::LogicParameter{false});
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_creatingAnimationParameter, DCore::LogicParameter{true});	
		return;
	case createEndEnd:
		OutOfThrowAttack();
		IntoIdleState();
		return;
	case faceAttackBeginPrepareEnd:
		if (m_stateContext.StateUnion.FaceAttackContext.AttackUp)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackBeginPrepareAnimationParameter, DCore::LogicParameter{ false });
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackPreparingAnimationParameter, DCore::LogicParameter{ true });
			return;
		}
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackBeginPrepareAnimationParameter, DCore::LogicParameter{ false });
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackPreparingAnimationParameter, DCore::LogicParameter{ true });
		return;
	case faceAttackPreparingEnd:
		m_stateContext.StateUnion.FaceAttackContext.CurrentNumberOfPreparationLoops++;
		if (m_stateContext.StateUnion.FaceAttackContext.CurrentNumberOfPreparationLoops >= m_stateContext.StateUnion.FaceAttackContext.NumberOfPreparationLoops)
		{
			if (m_stateContext.StateUnion.FaceAttackContext.AttackUp)
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackPreparingAnimationParameter, DCore::LogicParameter{ false });
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackBeginAttackAnimationParameter, DCore::LogicParameter{ true });
				return;
			}
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackPreparingAnimationParameter, DCore::LogicParameter{ false });
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackBeginAttackAnimationParameter, DCore::LogicParameter{ true });
		}
		return;
	case faceAttackBeginAttackEnd:
		m_faceAttackLoopSound.Start();
		if (m_stateContext.StateUnion.FaceAttackContext.AttackUp)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackBeginAttackAnimationParameter, DCore::LogicParameter{ false });
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackAttackingAnimationParameter, DCore::LogicParameter{ true });	
			return;
		}
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackBeginAttackAnimationParameter, DCore::LogicParameter{ false });
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackAttackingAnimationParameter, DCore::LogicParameter{ true });		
		return;
	case faceAttackAttackingEnd:
		m_stateContext.StateUnion.FaceAttackContext.CurrentNumberOfAttackLoops++;
		if (m_stateContext.StateUnion.FaceAttackContext.CurrentNumberOfAttackLoops >= m_stateContext.StateUnion.FaceAttackContext.NumberOfAttackLoops)
		{
			if (m_stateContext.StateUnion.FaceAttackContext.AttackUp)
			{
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackAttackingAnimationParameter, DCore::LogicParameter{ false });
				m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackEndAttackAnimationParameter, DCore::LogicParameter{ true });
				return;
			}
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackAttackingAnimationParameter, DCore::LogicParameter{ false });
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackEndAttackAnimationParameter, DCore::LogicParameter{ true });
		}
		return;
	case faceAttackEndAttackEnd:
		OutOfFaceAttack();
		IntoIdleState();
		return;
	case upFaceAttackBeginAttackFrame0:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginAttackBoxColliderParams[0]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackBeginAttackFrame1:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginAttackBoxColliderParams[1]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackBeginAttackFrame2:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginAttackBoxColliderParams[2]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackBeginAttackFrame3:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginAttackBoxColliderParams[3]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackBeginAttackFrame4:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginAttackBoxColliderParams[4]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackAttackingFrame:
	{
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({m_upFaceAttackAttackingBoxColliderParams.Translation.x, m_upFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(m_upFaceAttackAttackingBoxColliderParams.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({m_upFaceAttackAttackingBoxColliderParams.Translation.x, m_upFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		m_hurtBoxComponent.SetSizes(m_upFaceAttackAttackingBoxColliderParams.Sizes);
		return;
	}
	case upFaceAttackEndAttackFrame0:
	{
		const BoxColliderParams& params(m_upFaceAttackEndAttackBoxColliderParams[0]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		m_faceAttackEndSound.Start();
		return;
	}
	case upFaceAttackEndAttackFrame1:
	{
		const BoxColliderParams& params(m_upFaceAttackEndAttackBoxColliderParams[1]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackEndAttackFrame2:
	{
		const BoxColliderParams& params(m_upFaceAttackEndAttackBoxColliderParams[2]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackEndAttackFrame3:
	{
		const BoxColliderParams& params(m_upFaceAttackEndAttackBoxColliderParams[3]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackEndAttackFrame4:
	{
		const BoxColliderParams& params(m_upFaceAttackEndAttackBoxColliderParams[4]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackBeginPrepareFrame0:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginPrepareBoxColliderParams[0]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		m_faceAttackBeginSound.Start();
		return;
	}
	case upFaceAttackBeginPrepareFrame1:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginPrepareBoxColliderParams[1]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackBeginPrepareFrame2:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginPrepareBoxColliderParams[2]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case upFaceAttackBeginPrepareFrame3:
	{
		const BoxColliderParams& params(m_upFaceAttackBeginPrepareBoxColliderParams[3]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginPrepareFrame0:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginPrepareBoxColliderParams[0]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		m_faceAttackBeginSound.Start();
		return;
	}
	case downFaceAttackBeginPrepareFrame1:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginPrepareBoxColliderParams[1]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginPrepareFrame2:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginPrepareBoxColliderParams[2]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginPrepareFrame3:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginPrepareBoxColliderParams[3]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginAttackFrame0:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginAttackBoxColliderParams[0]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginAttackFrame1:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginAttackBoxColliderParams[1]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginAttackFrame2:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginAttackBoxColliderParams[2]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginAttackFrame3:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginAttackBoxColliderParams[3]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackBeginAttackFrame4:
	{
		const BoxColliderParams& params(m_downFaceAttackBeginAttackBoxColliderParams[4]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({ params.Translation.x, params.Translation.y, 0.0f });
		m_hurtBoxComponent.SetSizes(params.Sizes);
		return;
	}
	case downFaceAttackAttackingFrame:
	{
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({m_downFaceAttackAttackingBoxColliderParams.Translation.x, m_downFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(m_downFaceAttackAttackingBoxColliderParams.Sizes);		
		m_hurtBoxTransformComponent.SetTranslation({m_downFaceAttackAttackingBoxColliderParams.Translation.x, m_downFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		m_hurtBoxComponent.SetSizes(m_downFaceAttackAttackingBoxColliderParams.Sizes);
		return;
	}
	case downFaceAttackEndAttackFrame0:
	{
		const BoxColliderParams& params(m_downFaceAttackEndAttackBoxColliderParams[0]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({m_downFaceAttackAttackingBoxColliderParams.Translation.x, m_downFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		m_hurtBoxComponent.SetSizes(m_downFaceAttackAttackingBoxColliderParams.Sizes);
		m_faceAttackEndSound.Start();
		return;
	}
	case downFaceAttackEndAttackFrame1:
	{
		const BoxColliderParams& params(m_downFaceAttackEndAttackBoxColliderParams[1]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({m_downFaceAttackAttackingBoxColliderParams.Translation.x, m_downFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		m_hurtBoxComponent.SetSizes(m_downFaceAttackAttackingBoxColliderParams.Sizes);
		return;
	}
	case downFaceAttackEndAttackFrame2:
	{
		const BoxColliderParams& params(m_downFaceAttackEndAttackBoxColliderParams[2]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);
		m_hurtBoxTransformComponent.SetTranslation({m_downFaceAttackAttackingBoxColliderParams.Translation.x, m_downFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		m_hurtBoxComponent.SetSizes(m_downFaceAttackAttackingBoxColliderParams.Sizes);
		return;
	}
	case downFaceAttackEndAttackFrame3:
	{
		const BoxColliderParams& params(m_downFaceAttackEndAttackBoxColliderParams[3]);
		auto [transformComponent, boxCollidercomponent] = m_attackHitBox.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
		transformComponent.SetTranslation({params.Translation.x, params.Translation.y, 0.0f});
		boxCollidercomponent.SetSizes(params.Sizes);	
		m_hurtBoxTransformComponent.SetTranslation({m_downFaceAttackAttackingBoxColliderParams.Translation.x, m_downFaceAttackAttackingBoxColliderParams.Translation.y, 0.0f});
		m_hurtBoxComponent.SetSizes(m_downFaceAttackAttackingBoxColliderParams.Sizes);
		return;
	}
	case gunAttackBeginAttackEnd:
	{
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackBeginAttackAnimationParameter, DCore::LogicParameter{false});
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackAttackingAnimationParameter, DCore::LogicParameter{true});
		return;
	}
	case gunAttackAttackingEnd:
	{
		m_stateContext.StateUnion.GunAttackContext.CurrentNumberOfAttackingLoops++;
		if (m_stateContext.StateUnion.GunAttackContext.CurrentNumberOfAttackingLoops >= m_stateContext.StateUnion.GunAttackContext.NumberOfAttackingLoops)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackAttackingAnimationParameter, DCore::LogicParameter{ false });
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackEndAttackAnimationParameter, DCore::LogicParameter{ true });
		}
		return;
	}
	case gunAttackEndAttackEnd:
	{
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackEndAttackAnimationParameter, DCore::LogicParameter{ false });
		OutOfGunAttack();
		IntoIdleState();
		return;
	}
	case idleBegin:
		m_hurtBoxTransformComponent.SetTranslation({4.394f, 2.872f, 0.0f});
		m_hurtBoxComponent.SetSizes({ 3.0f, 3.0f });
		return;
	case gunAttackBegin:
		m_hurtBoxTransformComponent.SetTranslation({4.52f, 2.661f, 0.0f});
		m_hurtBoxComponent.SetSizes({ 2.0f, 3.0f });
		m_gunAttackBeginSound.Start();
		return;
	case gunAttackAttackingBegin:
		m_seedMissilesSpriteComponent.SetEnabled(true);
		m_seedMissilesAsmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_seedMissilesFireAnimationParameter, DCore::LogicParameter{ true });
		if (!m_gunAttackLoopSound.IsPlaying())
		{
			m_gunAttackLoopSound.Start();
		}
		return;
	case gunAttackAttackEndBegin:
		m_seedMissilesSpriteComponent.SetEnabled(false);
		m_seedMissilesAsmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_seedMissilesFireAnimationParameter, DCore::LogicParameter{ false });	
		m_gunAttackLoopSound.Stop();
		m_gunAttackEndSound.Start();
		return;
	case intro1End:
		if (m_stateContext.StateUnion.IntroStateContext.CurrentNumberOfIntro1++ >= m_stateContext.StateUnion.IntroStateContext.NumberOfIntro1)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_intro1AnimationParameter, DCore::LogicParameter{ false });
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_intro2AnimationParameter, DCore::LogicParameter{ true });
			m_introSound.Start();
			return;
		}
		return;
	case intro2End:
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_intro2AnimationParameter, DCore::LogicParameter{ false });
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introLoopAnimationParameter, DCore::LogicParameter{ true });
		return;
	case introLoopEnd:
		if (m_stateContext.StateUnion.IntroStateContext.CurrentNumberOfIntroLoops++ >= m_stateContext.StateUnion.IntroStateContext.NumberOfIntroLoops)
		{
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introLoopAnimationParameter, DCore::LogicParameter{ false });
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introEndAnimationParameter, DCore::LogicParameter{ true });
			return;
		}
		return;
	case introEndEnd:
		OutOfIntroState();
		IntoIdleState();
		return;
	case createBegin:
		if (m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount == 0)
		{
			m_createSound.Start();
		}
		return;
	default:
		 return;
	}
}

void FlowerComponent::OnOverlapBegin(DCore::EntityRef entity)
{
	DCore::ComponentRef<DCore::Component> playerShotComponentRef(entity.GetComponent(DCore::ComponentId::GetId<PlayerShotComponent>()));
	PlayerShotComponent* playerShotComponent(static_cast<PlayerShotComponent*>(playerShotComponentRef.GetRawComponent()));
	playerShotComponent->Kill();
	m_currentHealth -= 1;
	if (m_currentHealth == 0)
	{
		m_gunAttackLoopSound.Stop();
		m_knockoutSound.Start();
		OutOfThrowAttack();
		OutOfFaceAttack();
		OutOfGunAttack();
		IntoDeathState();
		m_seedMissilesSpriteComponent.SetEnabled(false);
		m_seedMissilesAsmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_seedMissilesFireAnimationParameter, DCore::LogicParameter{ false });
		const DCore::ComponentIdType playerComponentId(DCore::ComponentId::GetId<PlayerComponent>());
		IterateOnEntitiesWithComponents(
			&playerComponentId, 1,
			[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
			{
				PlayerComponent* playerComponent(static_cast<PlayerComponent*>(component.GetRawComponent()));
				playerComponent->FlowerDefeated();
				return true;
			});
		const DCore::ComponentIdType knockoutComponentId(DCore::ComponentId::GetId<KnockoutComponent>());
		IterateOnEntitiesWithComponents(
			&knockoutComponentId, 1,
			[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
			{
				KnockoutComponent* knockoutComponent(static_cast<KnockoutComponent*>(component.GetRawComponent()));
				knockoutComponent->DisplayKnockoutMessage();
				return true;
			});	
		const DCore::ComponentIdType gameManagerComponentId(DCore::ComponentId::GetId<GameManagerComponent>());
		IterateOnEntitiesInAllScenesWithComponents(
			&gameManagerComponentId, 1,
			[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
			{
				GameManagerComponent* gameManagerComponent(static_cast<GameManagerComponent*>(component.GetRawComponent()));
				gameManagerComponent->BossDied();
				return true;
			});		
	}
}

void FlowerComponent::PlayerDied()
{
	m_playerDied = true;
}

void FlowerComponent::GetAnimationParametersIndexes()
{
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Projectiles", m_projectilesAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Create Begin", m_createBeginAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Creating", m_creatingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Throw Begin", m_throwBeginAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Throwing", m_throwingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Create Reset", m_createResetAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Create End", m_createEndAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Up Face Attack", m_upFaceAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("UFA_BeginPrepare", m_upFaceAttackBeginPrepareAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("UFA_Preparing", m_upFaceAttackPreparingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("UFA_BeginAttack", m_upFaceAttackBeginAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("UFA_Attacking", m_upFaceAttackAttackingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("UFA_EndAttack", m_upFaceAttackEndAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Down Face Attack", m_downFaceAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("DFA_BeginPrepare", m_downFaceAttackBeginPrepareAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("DFA_Preparing", m_downFaceAttackPreparingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("DFA_BeginAttack", m_downFaceAttackBeginAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("DFA_Attacking", m_downFaceAttackAttackingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("DFA_EndAttack", m_downFaceAttackEndAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Gun Attack", m_gunAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Gun Begin Attack", m_gunAttackBeginAttackAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Gun Attacking", m_gunAttackAttackingAnimationParameter);
	m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Gun End Attack", m_gunAttackEndAttackAnimationParameter);
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Trigger>("Death", m_deathAnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro", m_introAnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro1", m_intro1AnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro2", m_intro2AnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro Loop", m_introLoopAnimationParameter));
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Intro End", m_introEndAnimationParameter));
}

void FlowerComponent::GetFaceAttackHitBoxes()
{
	m_entityRef.IterateOnChildren
	(
		[&](DCore::EntityRef entity) -> bool
		{
			DCore::DString entityName;
			entity.GetName(entityName);
			if (entityName == "Attack Hit Box")
			{
				m_attackHitBox = entity;
				return false;
			}
			return false;
		}
	);
	DASSERT_E(m_attackHitBox.IsValid());
}

void FlowerComponent::SetupHurtBox()
{
	m_entityRef.IterateOnChildren
	(
		[&](DCore::EntityRef child) -> bool
		{
			DCore::DString name;
			child.GetName(name);
			if (name == "Hurt Box")
			{
				auto [transformComponent, boxColliderComponent] = child.GetComponents<DCore::TransformComponent, DCore::BoxColliderComponent>();
				m_hurtBoxTransformComponent = transformComponent;
				m_hurtBoxComponent = boxColliderComponent;
				DASSERT_E(m_hurtBoxTransformComponent.IsValid() && m_hurtBoxComponent.IsValid());
				m_hurtBoxBodyId = m_hurtBoxComponent.GetBodyId();
				m_shotOverlapBeginRegistrationIndex = m_runtime->RegisterToOnOverlapBegin(GenerateComponentRefConstructorArgs(), m_hurtBoxBodyId);
			}
			return false;
		}
	);
}

void FlowerComponent::IntoIdleState()
{
	float random(std::generate_canonical<float, 10>(s_randomGenerator) + m_maximumAttackCooldownOffset);
	m_stateContext.Type = StateContextType::Idle;
	m_stateContext.StateUnion.IdleStateContext.TimeInIdleState = m_minimumAttackCooldownTime + random;
	m_stateContext.StateUnion.IdleStateContext.CurrentTimeInIdleState = m_stateContext.StateUnion.IdleStateContext.TimeInIdleState;
}

void FlowerComponent::IntoThrowAttack()
{
	constexpr size_t numberOfCreatingAnimationLoops(2);
	constexpr size_t numberOfThrowingAnimationLoops(10);
	m_stateContext.Type = StateContextType::Throw;
	static std::discrete_distribution numberOfAttacksDistribution({1, 1, 1});
	static std::discrete_distribution isFirstAttackBoomerangDistribution({1, 1});
	m_stateContext.StateUnion.ThrowAttackContext.NumberOfAttacks = numberOfAttacksDistribution(s_randomGenerator) + 1;
	m_stateContext.StateUnion.ThrowAttackContext.CurrentAttackCount = 0;
	m_stateContext.StateUnion.ThrowAttackContext.IsFirstBoomerang = isFirstAttackBoomerangDistribution(s_randomGenerator);
	m_stateContext.StateUnion.ThrowAttackContext.NumberOfCreatingAnimationLoops = numberOfCreatingAnimationLoops;
	m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfCreatingAnimationLoops = 0;
	m_stateContext.StateUnion.ThrowAttackContext.NumberOfThrowingAnimationLoops = numberOfThrowingAnimationLoops;
	m_stateContext.StateUnion.ThrowAttackContext.CurrentNumberOfThrowingAnimationLoops = 0;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_projectilesAnimationParameter, DCore::LogicParameter{true});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createBeginAnimationParameter, DCore::LogicParameter{true});
}

void FlowerComponent::OutOfThrowAttack()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_projectilesAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createBeginAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_creatingAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_throwBeginAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_throwingAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createResetAnimationParameter, DCore::LogicParameter{false});
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_createEndAnimationParameter, DCore::LogicParameter{false});
}

void FlowerComponent::LaunchAcorns()
{
	AcornManagerComponent* acornManagerComponent(static_cast<AcornManagerComponent*>(m_acornManagerComponent.GetRawComponent()));
	acornManagerComponent->LaunchAcorns();
}

void FlowerComponent::LaunchBoomerang()
{
	BoomerangManagerComponent* boomerangManagerComponent(static_cast<BoomerangManagerComponent*>(m_boomerangManagerComponent.GetRawComponent()));
	boomerangManagerComponent->Launch();
}

void FlowerComponent::IntoFaceAttack()
{
	constexpr size_t numberOfPreparationLoops(8);
	constexpr size_t numberOfAttackLoops(4);
	static std::discrete_distribution attackUpDistribution({1, 1});
	m_stateContext.Type = StateContextType::FaceAttack;
	m_stateContext.StateUnion.FaceAttackContext.NumberOfPreparationLoops = numberOfPreparationLoops;
	m_stateContext.StateUnion.FaceAttackContext.CurrentNumberOfPreparationLoops = 0;
	m_stateContext.StateUnion.FaceAttackContext.NumberOfAttackLoops = numberOfAttackLoops;
	m_stateContext.StateUnion.FaceAttackContext.CurrentNumberOfAttackLoops = 0;
	m_stateContext.StateUnion.FaceAttackContext.AttackUp = static_cast<bool>(attackUpDistribution(s_randomGenerator));
	if (m_stateContext.StateUnion.FaceAttackContext.AttackUp)
	{
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackAnimationParameter, DCore::LogicParameter{ true });
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackBeginPrepareAnimationParameter, DCore::LogicParameter{ true });
		return;
	}
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackAnimationParameter, DCore::LogicParameter{ true });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackBeginPrepareAnimationParameter, DCore::LogicParameter{ true });
}

void FlowerComponent::OutOfFaceAttack()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackBeginPrepareAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackPreparingAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackBeginAttackAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackAttackingAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_upFaceAttackEndAttackAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackBeginPrepareAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackPreparingAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackBeginAttackAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackAttackingAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_downFaceAttackEndAttackAnimationParameter, DCore::LogicParameter{ false });
}

void FlowerComponent::IntoGunAttack()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackAnimationParameter, DCore::LogicParameter{ true });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackBeginAttackAnimationParameter, DCore::LogicParameter{ true });
	m_stateContext.Type = StateContextType::GunAttack;
	constexpr size_t numberOfAttackingLoops(17);
	m_stateContext.StateUnion.GunAttackContext.NumberOfAttackingLoops = numberOfAttackingLoops;
	m_stateContext.StateUnion.GunAttackContext.CurrentNumberOfAttackingLoops = 0;
	SeedEnemiesLauncherComponent* seedEnemiesLauncherComponent(static_cast<SeedEnemiesLauncherComponent*>(m_seedEnemiesLauncherComponent.GetRawComponent()));
	seedEnemiesLauncherComponent->LaunchSeeds();
}

void FlowerComponent::OutOfGunAttack()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackBeginAttackAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackAttackingAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_gunAttackEndAttackAnimationParameter, DCore::LogicParameter{ false });
}

void FlowerComponent::IntoDeathState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Trigger>(m_deathAnimationParameter, DCore::TriggerParameter{ true });
}

void FlowerComponent::IntoIntroState()
{
	m_stateContext.Type = StateContextType::Intro;
	constexpr size_t numberOfIntro1(3);
	constexpr size_t numberOfIntroLoops(5);
	m_stateContext.StateUnion.IntroStateContext.NumberOfIntro1 = numberOfIntro1;
	m_stateContext.StateUnion.IntroStateContext.CurrentNumberOfIntro1 = 0;
	m_stateContext.StateUnion.IntroStateContext.NumberOfIntroLoops = numberOfIntroLoops;
	m_stateContext.StateUnion.IntroStateContext.CurrentNumberOfIntroLoops = 0;
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introAnimationParameter, DCore::LogicParameter{ true });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_intro1AnimationParameter, DCore::LogicParameter{ true });
}

void FlowerComponent::OutOfIntroState()
{
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_intro1AnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_intro2AnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introLoopAnimationParameter, DCore::LogicParameter{ false });
	m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_introEndAnimationParameter, DCore::LogicParameter{ false });
}

}
