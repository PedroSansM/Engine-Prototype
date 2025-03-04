#pragma once

#include "DommusCore.h"



namespace Game
{
	class GameManagerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::GameManagerComponent>
{
	ConstructorArgs()
	{}
	
	float BeginTransitionToMainMenuDelay;
	float TransitionToMainMenuDuration;
};
#pragma pack(pop)

namespace Game
{

class GameManagerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_beginTransitionToMainMenuDelay{0};
	static constexpr size_t a_transitionToMainMenuDuration{1};
public:
	GameManagerComponent(const DCore::ConstructorArgs<GameManagerComponent>&);
	~GameManagerComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
public:
	void StartGame();
	void PlayerDied();
	void BossDied();
private:
	// Editor
	float m_beginTransitionToMainMenuDelay;
	float m_transitionToMainMenuDuration;
	// Runtime
	DCore::ComponentRef<DCore::Component> m_hourglassComponent;
	DCore::ComponentRef<DCore::Component> m_musicManagerComponent;
	DCore::ComponentRef<DCore::Component> m_mainMenuControllerComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_backgroundSpriteComponent;
	bool m_playerDied;
	bool m_isGameLoading;
	bool m_bossDied;
	float m_currentBeginTransitionToMainMenuDelay;
	float m_currentTransitionToMainMenuDuration;
};

class GameManagerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~GameManagerComponentScriptComponentFormGenerator() = default;
private:
	GameManagerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"GameManagerComponent",
			DCore::ComponentId::GetId<GameManagerComponent>(),
			sizeof(GameManagerComponent),
			sizeof(DCore::ConstructorArgs<GameManagerComponent>),
			{{DCore::AttributeName("Begin Transition To Main Menu Delay"), DCore::AttributeType::Float, GameManagerComponent::a_beginTransitionToMainMenuDelay},
			{DCore::AttributeName("Transition To Main Menu Duration"), DCore::AttributeType::Float, GameManagerComponent::a_transitionToMainMenuDuration}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) GameManagerComponent(*static_cast<const DCore::ConstructorArgs<GameManagerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<GameManagerComponent*>(componentAddress)->~GameManagerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<GameManagerComponent> m_defaultArgs;
private:
	static GameManagerComponentScriptComponentFormGenerator s_generator;
};

}
	