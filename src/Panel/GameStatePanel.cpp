#include "GameStatePanel.h"
#include "ConsolePanel.h"
#include "TextureManager.h"
#include "ProgramContext.h"
#include "SceneManager.h"
#include "ConfigurationPanel.h"
#include "Log.h"
#include "SceneHierarchyPanel.h"

#include "DommusCore.h"

#include "imgui.h"

#include <iostream>



namespace DEditor
{

GameStatePanel::GameStatePanel()
	:
	m_playButtonIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture"  / "PlayIcon.png")),
	m_gameState(GameState::NotPlaying)
{}

void GameStatePanel::Render()
{
//if (!ImGui::Begin("ControlGameState", nullptr))
//{
//	ImGui::End();
//	return;
//}
//ImGui::ImageButton("##PlayButton", (ImTextureID)(intptr_t)m_playButtonIcon.GetId(), ImVec2(32.0f, 32.0f), {0, 1}, {1, 0});
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_P))
	{
		switch (m_gameState)
		{
		case GameState::NotPlaying:
			if (!DCore::GlobalConfig::Get().IsStartingSceneDefined())
			{
				Log::Get().TerminalLog("Starting scene is not defined.");
				Log::Get().ConsoleLog(LogLevel::Error, "%s", "Starting scene is not defined");
				break;
			}
			m_gameState = GameState::Playing;
			break;
		case GameState::Playing:
			m_gameState = GameState::NotPlaying;
			break;
		case GameState::Paused:
			m_gameState = GameState::NotPlaying;
			break;
		}
	}
//ImGui::End();
}

}
