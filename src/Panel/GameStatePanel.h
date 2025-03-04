#pragma once

#include "Panel.h"

#include "DommusCore.h"

#include <vector>



namespace DEditor
{

enum class GameState
{
	NotPlaying,
	Playing,
	Paused
};

class GameStatePanel : public Panel
{
public:
	GameStatePanel(const GameStatePanel&) = delete;
	GameStatePanel(GameStatePanel&&) =  delete;
	~GameStatePanel() = default;
public:
	static GameStatePanel& Get()
	{
		static GameStatePanel controlGameStatePanel;
		return controlGameStatePanel;
	}
public:
	void Render();
public:
	GameState GetGameState() const
	{
		return m_gameState;
	}
private:
	GameStatePanel();
private:
	DCore::Texture2D m_playButtonIcon;
	GameState m_gameState;
	DCore::SceneAssetManager::sceneContainerType m_editorScenes;
	std::unordered_map<DCore::UUIDType, DCore::InternalSceneRefType> m_editorLoadedScenes;
};

}
