#include "UtilityPanel.h"
#include "Panels.h"
#include "SceneHierarchyPanel.h"
#include "InspectorPanel.h"
#include "ResourcesPanel.h"
#include "ConsolePanel.h"
#include "MaterialManager.h"
#include "EditorGameViewPanels.h"
#include "GameViewPanel.h"
#include "ConfigurationPanel.h"
#include "SceneManager.h"
#include "GameStatePanel.h"
#include "Log.h"

#include "imgui.h"

#include <cstdio>



namespace DEditor
{

void UtilityPanel::Render()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::MenuItem("Save"))
			{
				if (GameStatePanel::Get().GetGameState() == GameState::NotPlaying)
				{
					SceneManager::Get().SaveLoadedScenes();
				}
				else
				{
					Log::Get().TerminalLog("%s", "Cannot save scenes while the game is playing.");
					Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot save scenes while the game is playing.");
				}
			}
			if (ImGui::MenuItem("Configuration"))
			{
				ConfigurationPanel::Get().Open();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Scene Hierarchy"))	
			{
				SceneHierarchyPanel::Get().Open();
			}
			if (ImGui::MenuItem("Inspector"))
			{
				InspectorPanel::Get().Open();
			}
			if (ImGui::MenuItem("Resources"))
			{
				ResourcesPanel::Get().Open();
			}
			if (ImGui::MenuItem("Console"))
			{
				ConsolePanel::Get().Open();
			}
			if (ImGui::MenuItem("Game viewport"))
			{
				GameViewPanel::Get().Open();
			}
			if (ImGui::BeginMenu("Viewports"))
			{
				for (size_t i(0); i < EditorGameViewPanels::numberOfPanels; i++)
				{
					char viewportName[11];
					std::sprintf(viewportName, "viewport %zu", i);
					if (ImGui::MenuItem(viewportName))
					{
						EditorGameViewPanels::Get().Open(i);
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

}
