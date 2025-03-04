#pragma once


#include "PlayerComponent.h"

#include "DommusCore.h"

#include "Log.h"



namespace Game
{
	class PlayerAnimationComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::PlayerAnimationComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class PlayerAnimationComponent : public DCore::ScriptComponent
{
public:
public:
	PlayerAnimationComponent(const DCore::ConstructorArgs<PlayerAnimationComponent>&)
	{}
	~PlayerAnimationComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override
	{
		DCore::EntityRef parent;
		if (m_entityRef.TryGetParent(parent) && parent.HaveComponents<PlayerComponent>())
		{
			m_playerComponent = parent.GetComponent(DCore::ComponentId::GetId<PlayerComponent>());
			return;
		}
		DEditor::Log::Get().ConsoleLog(DEditor::LogLevel::Warning, "%s", "Fail to get player component in player animation component.");
	}

	virtual void OnMetachannelEvent(size_t metachannelId) override
	{
		constexpr size_t dashEndAnimationEventId(0);
		constexpr size_t shootStraightEventId(1);
		constexpr size_t shootDownEventId(2);
		constexpr size_t shootUpEventId(3);
		constexpr size_t shootDiagonalDownEventId(4);
		constexpr size_t shootDiagonalUpEventId(5);
		constexpr size_t duckBeginEndEventId(6);
		constexpr size_t shootInAirEventId(7);
		constexpr size_t shootDuckEventId(8);
		constexpr size_t hitBeginEventId(9);
		constexpr size_t hitLoopEventId(10);
		constexpr size_t hitEndEventId(11);
		constexpr size_t introBeginEnd(12);
		constexpr size_t introLoopEnd(13);
		constexpr size_t introEndEnd(14);
		PlayerComponent* playerComponent(static_cast<PlayerComponent*>(m_playerComponent.GetRawComponent()));
		switch (metachannelId)
		{
		case dashEndAnimationEventId:
			playerComponent->OnDashEnd();
			return;
		case shootStraightEventId:
			playerComponent->ShootStraight();
			return;
		case shootDownEventId:
			playerComponent->ShootDown();
			return;
		case shootUpEventId:
			playerComponent->ShootUp();
			return;
		case shootDiagonalDownEventId:
			playerComponent->ShootDiagonalDown();
			return;
		case shootDiagonalUpEventId:
			playerComponent->ShootDiagonalUp();
			return;
		case duckBeginEndEventId:
			playerComponent->OnDuckBeginEnd();
			return;
		case shootInAirEventId:
			playerComponent->ShootInAir();
			return;
		case shootDuckEventId:
			playerComponent->ShootDuck();
			return;
		case hitBeginEventId:
			playerComponent->ToEndHitBegin();
			return;
		case hitLoopEventId:
			playerComponent->ToEndHitLoop();
			return;
		case hitEndEventId:
			playerComponent->ToEndHitEnd();
			return;
		case introBeginEnd:
			playerComponent->IntroBeginEnd();
			return;
		case introLoopEnd:
			playerComponent->IntroLoopEnd();
			return;
		case introEndEnd:
			playerComponent->IntroEndEnd();
			return;
		default:
			return;
		}
	}
private:
	DCore::ComponentRef<DCore::Component> m_playerComponent;
};

class PlayerAnimationComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~PlayerAnimationComponentScriptComponentFormGenerator() = default;
private:
	PlayerAnimationComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"PlayerAnimationComponent",
			DCore::ComponentId::GetId<PlayerAnimationComponent>(),
			sizeof(PlayerAnimationComponent),
			sizeof(DCore::ConstructorArgs<PlayerComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) PlayerAnimationComponent(*static_cast<const DCore::ConstructorArgs<PlayerAnimationComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<PlayerAnimationComponent*>(componentAddress)->~PlayerAnimationComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<PlayerAnimationComponent> m_defaultArgs;
private:
	static PlayerAnimationComponentScriptComponentFormGenerator s_generator;
};

}
	
