#include "ConfigurationPanel.h"
#include "GameStatePanel.h"
#include "SceneManager.h"
#include "GlobalConfigurationSerializer.h"
#include "Log.h"
#include "FileBrowser.h"
#include "ProgramContext.h"

#include "imgui.h"

#include <cstdarg>
#include <filesystem>



namespace DEditor
{

ConfigurationPanel::ConfigurationPanel()
	:
	m_isOpened(false),
	m_windowFlags(ImGuiWindowFlags_None)
{
	if (DCore::GlobalConfig::Get().IsStartingSceneDefined())
	{
		SceneManager::Get().GetSceneName(DCore::GlobalConfig::Get().GetStartingSceneUUID(), m_startingSceneName);
	}
}

void ConfigurationPanel::Render()
{
	if (!m_isOpened)
	{
		return;
	}
	if (!ImGui::Begin("Configuration", &m_isOpened, m_windowFlags))
	{
		ImGui::End();
		return;
	}
	if (ImGui::Button("Save"))
	{
		m_windowFlags = ImGuiWindowFlags_None;
		GlobalConfigurationSerializer::Get().Save();
	}
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", "Starting scene:");
	ImGui::SameLine();
	if (m_startingSceneName.empty())
	{
		// TODO: Make a search mechanism for created scenes.
		ImGui::Button("No starting scene defined");
	}
	else
	{
		ImGui::Button(m_startingSceneName.c_str());
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload(ImGui::AcceptDragDropPayload("SCENE_UUID"));
		if (payload != nullptr)
		{
			const uuidType startingSceneUUID(*static_cast<uuidType*>(payload->Data));
			GlobalConfigurationSerializer::Get().SetStartingSceneUUID(startingSceneUUID);
			SceneManager::Get().GetSceneName(startingSceneUUID, m_startingSceneName);
			SetPanelToUnsavedState();
		}
	}
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Target Aspect Ratio");
	ImGui::SameLine();
	if (ImGui::BeginCombo("##TargetAspectRatio", DCore::AspectRatioUtilities::Get().GetAspectRatioString(DCore::GlobalConfig::Get().GetTargetAspectRatio())))
	{
		for (size_t i(0); i < DCore::AspectRatioUtilities::numberOfAspectRatios; i++)
		{
			const DCore::AspectRatio aspectRatio(static_cast<DCore::AspectRatio>(i));
			if (ImGui::Selectable(DCore::AspectRatioUtilities::Get().GetAspectRatioString(aspectRatio)))
			{
				GlobalConfigurationSerializer::Get().SetTargetAspectRatio(aspectRatio);
				SetPanelToUnsavedState();
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::CollapsingHeader("Sound"))
	{
		if (ImGui::TreeNodeEx("Banks", ImGuiTreeNodeFlags_SpanFullWidth))
		{
			if (ImGui::Button("Update"))
			{
				if (GameStatePanel::Get().GetGameState() == GameState::NotPlaying)
				{
					GlobalConfigurationSerializer::Get().UpdateSoundBanks();
				}
				else
				{
					Log::Get().TerminalLog("%s", "Sound banks can only be updated while the game is not playing.");
					Log::Get().ConsoleLog(LogLevel::Error, "%s", "Sound banks can only be updated while the game is not playing.");
				}
			}
			ImGui::Indent();
			GlobalConfigurationSerializer::Get().IterateOnBanksNames
			(
				[&](const stringType& bankName) -> bool
				{
					ImGui::Text("%s", bankName.c_str());
					return false;
				}
			);
			ImGui::Unindent();
			ImGui::TreePop();
		}
	}
	ImGui::End();
}

void ConfigurationPanel::SetPanelToUnsavedState()
{
	m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
}

}
