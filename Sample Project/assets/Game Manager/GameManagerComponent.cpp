#include "GameManagerComponent.h"
#include "HourglassComponent.h"
#include "MusicManagerComponent.h"
#include "MainMenuControllerComponent.h"

#include "Log.h"


namespace Game
{

GameManagerComponentScriptComponentFormGenerator GameManagerComponentScriptComponentFormGenerator::s_generator;

GameManagerComponent::GameManagerComponent(const DCore::ConstructorArgs<GameManagerComponent>& args)
	:
	m_beginTransitionToMainMenuDelay(args.BeginTransitionToMainMenuDelay),
	m_transitionToMainMenuDuration(args.TransitionToMainMenuDuration)
{}

void* GameManagerComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_beginTransitionToMainMenuDelay:
		return &m_beginTransitionToMainMenuDelay;
	case a_transitionToMainMenuDuration:
		return &m_transitionToMainMenuDuration;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void GameManagerComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_beginTransitionToMainMenuDelay:
		m_beginTransitionToMainMenuDelay = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_transitionToMainMenuDuration:
		m_transitionToMainMenuDuration = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	default:
		break;
	}
}

void GameManagerComponent::Start()
{
	m_playerDied = false;
	m_bossDied = false;
	m_isGameLoading = false;
	m_currentBeginTransitionToMainMenuDelay = m_beginTransitionToMainMenuDelay;
	m_currentTransitionToMainMenuDuration = m_transitionToMainMenuDuration;
	const DCore::ComponentIdType hourglassComponentId(DCore::ComponentId::GetId<HourglassComponent>());
	IterateOnEntitiesWithComponents(
		&hourglassComponentId, 1,
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			m_hourglassComponent = component;
			DASSERT_E(m_hourglassComponent.IsValid());
			return true;
		});
	const DCore::ComponentIdType musicManagerComponentId(DCore::ComponentId::GetId<MusicManagerComponent>());
	IterateOnEntitiesWithComponents(
		&musicManagerComponentId, 1,
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			m_musicManagerComponent = component;
			DASSERT_E(m_musicManagerComponent.IsValid());
			return true;
		});
	const DCore::ComponentIdType mainMenuControllerComponentId(DCore::ComponentId::GetId<MainMenuControllerComponent>());
	IterateOnEntitiesWithComponents(
		&mainMenuControllerComponentId, 1,
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			m_mainMenuControllerComponent = component;
			DASSERT_E(m_mainMenuControllerComponent.IsValid());
			return true;
		});
	m_entityRef.IterateOnChildren(
		[&](DCore::EntityRef child) -> bool
		{
			m_backgroundSpriteComponent = child.GetComponents<DCore::SpriteComponent>();
			DASSERT_E(m_backgroundSpriteComponent.IsValid());
			m_backgroundSpriteComponent.SetEnabled(false);
			return true;
		});
}

void GameManagerComponent::Update(float deltaTime)
{
	if (m_isGameLoading)
	{
		DCore::AssetManager::Get().IterateOnLoadedScenes(
			[&](DCore::SceneRef scene) -> bool
			{
				DCore::DString name;
				scene.GetName(name);
				if (name == "Game")
				{
					if (scene.IsLoaded())
					{
						m_isGameLoading = false;
						HourglassComponent* hourglassComponent(static_cast<HourglassComponent*>(m_hourglassComponent.GetRawComponent()));
						hourglassComponent->HideLoadingScreen();
						MusicManagerComponent* musicManagerComponent(static_cast<MusicManagerComponent*>(m_musicManagerComponent.GetRawComponent()));
						musicManagerComponent->PlayBossMusic();
					}
					return true;
				}
				return false;
			});
	}
	if (m_playerDied)
	{
		if (m_runtime->KeyPressed(DCore::DKey::R))
		{
			m_runtime->SetSceneToUnload("Game");
			m_runtime->SetSceneToLoadAsync("Game");
			HourglassComponent* hourglassComponent(static_cast<HourglassComponent*>(m_hourglassComponent.GetRawComponent()));
			hourglassComponent->DisplayLoadingScreen();
			m_playerDied = false;
			m_isGameLoading = true;
		}
	}
	else if (m_bossDied)
	{
		if (m_currentBeginTransitionToMainMenuDelay > 0.0f)
		{
			m_currentBeginTransitionToMainMenuDelay -= deltaTime;
			return;
		}
		m_backgroundSpriteComponent.SetEnabled(true);
		if (m_currentTransitionToMainMenuDuration > 0.0f)
		{
			const float nextAlpha(DCore::Math::Lerp(0.0f, 1.0f, (m_transitionToMainMenuDuration - m_currentTransitionToMainMenuDuration) / m_transitionToMainMenuDuration));
			m_backgroundSpriteComponent.SetTintColor({ 0.0f, 0.0f, 0.0f, nextAlpha });
			m_currentTransitionToMainMenuDuration -= deltaTime;
			if (m_currentTransitionToMainMenuDuration <= 0.0f)
			{
				m_playerDied = false;
				m_bossDied = false;
				m_runtime->SetSceneToUnload("Game");
				m_backgroundSpriteComponent.SetEnabled(false);
				MusicManagerComponent* musicManagerComponent(static_cast<MusicManagerComponent*>(m_musicManagerComponent.GetRawComponent()));
				musicManagerComponent->PlayMainMenuMusic();
				MainMenuControllerComponent* mainMenuControllerComponent(static_cast<MainMenuControllerComponent*>(m_mainMenuControllerComponent.GetRawComponent()));
				mainMenuControllerComponent->Enable();
				m_currentBeginTransitionToMainMenuDelay = m_beginTransitionToMainMenuDelay;
				m_currentTransitionToMainMenuDuration = m_transitionToMainMenuDuration;
			}
		}
	}
}

void GameManagerComponent::StartGame()
{
	if (m_isGameLoading)
	{
		return;
	}
	m_isGameLoading = true;
	HourglassComponent* hourglassComponent(static_cast<HourglassComponent*>(m_hourglassComponent.GetRawComponent()));
	hourglassComponent->DisplayLoadingScreen();
	MusicManagerComponent* musicManagerComponent(static_cast<MusicManagerComponent*>(m_musicManagerComponent.GetRawComponent()));
	musicManagerComponent->StopMainMenuMusic();
	m_runtime->SetSceneToLoadAsync("Game");
}

void GameManagerComponent::PlayerDied()
{
	MusicManagerComponent* musicManagerComponent(static_cast<MusicManagerComponent*>(m_musicManagerComponent.GetRawComponent()));
	musicManagerComponent->StopBossMusic();
	m_playerDied = true;
}

void GameManagerComponent::BossDied()
{
	m_bossDied = true;
	MusicManagerComponent* musicManagerComponent(static_cast<MusicManagerComponent*>(m_musicManagerComponent.GetRawComponent()));
	musicManagerComponent->StopBossMusic();
}

}