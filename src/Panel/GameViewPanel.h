#pragma once

#include "Panel.h"
#include "GameStatePanel.h"

#include "DommusCore.h"



namespace DEditor
{

class GameViewPanel : public Panel
{
public:
	using sceneContainerType = DCore::SceneAssetManager::sceneContainerType;
	using loadedSceneContainerType = DCore::SceneAssetManager::loadedSceneContainerType;
public:
	GameViewPanel(const GameViewPanel&) = delete;
	GameViewPanel(GameViewPanel&&) = delete;
	~GameViewPanel();	
public:
	static GameViewPanel& Get()
	{
		static GameViewPanel gameViewPanel;
		return gameViewPanel;
	}
public:
	void Open();
	void Render();
public:
	DCore::Runtime& GetRuntime()
	{
		return m_runtime;
	}
private:
	GameViewPanel();
private:
	bool m_isOpened;
	DCore::Runtime m_runtime;
	DCore::Renderer m_renderer;
	GameState m_currentGameState;
	sceneContainerType m_scenes;
	loadedSceneContainerType m_loadedScenes;
};

}
