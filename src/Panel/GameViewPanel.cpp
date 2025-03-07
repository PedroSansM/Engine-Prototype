#include "GameViewPanel.h"
#include "ConfigurationPanel.h"
#include "ProgramContext.h"
#include "GameStatePanel.h"

#include "SceneHierarchyPanel.h"
#include "SceneManager.h"
#include "imgui.h"



namespace DEditor
{

GameViewPanel::GameViewPanel()
	:
	m_isOpened(true),
	m_runtime(ProgramContext::Get().GetMainContext()),
	m_currentGameState(GameState::NotPlaying)
{
	m_renderer.Initiate(ProgramContext::Get().GetMainContext());
}

GameViewPanel::~GameViewPanel()
{
	if (m_isOpened)
	{
		m_renderer.Terminate();
	}
}

void GameViewPanel::Open()
{
	m_isOpened = true;
	if (m_currentGameState == GameState::NotPlaying)
	{
		m_renderer.Initiate(ProgramContext::Get().GetMainContext());
	}
}

void GameViewPanel::Render()
{
	if (!m_isOpened)
	{
		m_renderer.Terminate();
		return;
	}
	if (!ImGui::Begin("Game viewport", &m_isOpened))
	{
		ImGui::End();
		return;
	}
	const ImVec2 totalViewportSizes(ImGui::GetContentRegionAvail());
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, DCore::GlobalConfig::Get());
	const float aspectRatioValue(DCore::AspectRatioUtilities::Get().GetAspectRatioValue(DCore::GlobalConfig::Get().GetTargetAspectRatio()));
	ImVec2 rendererViewportSizes;
	if (aspectRatioValue >= 1.0f)
	{
		rendererViewportSizes.x = totalViewportSizes.x;
		rendererViewportSizes.y = rendererViewportSizes.x / aspectRatioValue;
		if (rendererViewportSizes.y > totalViewportSizes.y)
		{
			rendererViewportSizes.x /= (rendererViewportSizes.y / totalViewportSizes.y);
			rendererViewportSizes.y = totalViewportSizes.y;
		}
		ImVec2 cursorPosition(ImGui::GetCursorScreenPos());
		cursorPosition.x += (totalViewportSizes.x - rendererViewportSizes.x)/2.0f;
		cursorPosition.y += (totalViewportSizes.y - rendererViewportSizes.y)/2.0f;
		ImGui::SetCursorScreenPos(cursorPosition);
	}
	else if (aspectRatioValue == 0.0f)
	{
		rendererViewportSizes = totalViewportSizes;
	}
	// TODO. Handle portrait aspect ratios (0.0f < aspectRatio < 1.0f).
	switch (m_currentGameState)
	{
	case GameState::NotPlaying:
		if (GameStatePanel::Get().GetGameState() == GameState::Playing)
		{
			SceneHierarchyPanel::Get().ClearEntitySelection();
			DCore::AssetManager::Get().GetScenesInfo(m_scenes, m_loadedScenes);
			DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, DCore::GlobalConfig::Get());
			DCore::SceneRef scene;
			SceneManager::Get().LoadScene(DCore::GlobalConfig::Get().GetStartingSceneUUID(), &scene);
			scene.LoadingCompleted();
			m_runtime.Begin();
			m_currentGameState = GameState::Playing;
			break;
		}
		if (m_renderer.IsRenderingDone() && rendererViewportSizes.x > 0 && rendererViewportSizes.y > 0)
		{
			m_renderer.Begin({rendererViewportSizes.x, rendererViewportSizes.y});
			DCore::Runtime::MakeDefaultRendererSubmitions({rendererViewportSizes.x, rendererViewportSizes.y}, m_renderer);
			m_renderer.Render();
		}
		ImGui::Image((ImTextureID)(uintptr_t)m_renderer.GetOutputTextureId(), rendererViewportSizes, {0, 1}, {1, 0});
		break;	
	case GameState::Playing:
		if (GameStatePanel::Get().GetGameState() == GameState::NotPlaying)
		{
			m_runtime.End();
			DCore::AssetManager::Get().SetScenesInfo(std::move(m_scenes), std::move(m_loadedScenes));
			m_scenes.Clear();
			m_loadedScenes.clear();
			SceneHierarchyPanel::Get().ClearEntitySelection();
			m_currentGameState = GameState::NotPlaying;
			break;
		}
		if (m_renderer.IsRenderingDone() && rendererViewportSizes.x > 0 && rendererViewportSizes.y > 0)
		{
			m_runtime.Render({rendererViewportSizes.x, rendererViewportSizes.y}, m_renderer);
		}
		ImGui::Image((ImTextureID)(uintptr_t)m_renderer.GetOutputTextureId(), rendererViewportSizes, {0, 1}, {1, 0});
		break;
	default:
		break;
	}
	ImGui::End();
}

}
