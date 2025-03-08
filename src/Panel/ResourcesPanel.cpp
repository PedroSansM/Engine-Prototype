#include "ResourcesPanel.h"
#include "ConsolePanel.h"
#include "FileBrowser.h"
#include "Log.h"
#include "ProgramContext.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "EditorAssetManager.h"
#include "AnimationManager.h"
#include "SceneManager.h"
#include "InspectorPanel.h"
#include "GameStatePanel.h"
#include "AnimationPanel.h"
#include "AnimationStateMachineManager.h"
#include "AnimationStateMachinePanel.h"
#include "AnimationStateMachinePayload.h"
#include "PhysicsMaterialManager.h"
#include "PhysicsMaterialPanel.h"
#include "PhysicsMaterialPayload.h"
#include "SpriteMaterialPanel.h"
#include "TexturePanel.h"
#include "Window.h"
#include "SpriteSheetGenManager.h"
#include "SpriteSheetGenPanel.h"

#include "imgui_internal.h"
#include "yaml-cpp/yaml.h"
#include "imgui.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <cfloat>
#include <fstream>
#include <system_error>



namespace DEditor
{

// Resources Item
ResourcesPanel::ResourceItem::ResourceItem(
		DCore::DUInt id, 
		const DCore::DString& name, 
		DCore::DUInt thumbnailTextureId, 
		void (ResourcesPanel::*onClick)(ResourceItem&), 
		void (ResourcesPanel::*onDoubleClick)(ResourceItem&), 
		void (ResourcesPanel::*onRightClick)(ResourceItem&),
		void (ResourcesPanel::*onRender)(ResourceItem&))
	:
	m_id(id),
	m_name(name),
	m_thumbnailTextureId(thumbnailTextureId),
	OnClick(onClick),
	OnDoubleClick(onDoubleClick),
	OnRightClick(onRightClick),
	OnRender(onRender)
{}

ResourcesPanel::ResourceItem::ResourceItem(
		DCore::DUInt id, 
		const DCore::DString& name, 
		const DCore::UUIDType& uuid, 
		DCore::DUInt thumbnailTextureId, 
		void (ResourcesPanel::*onClick)(ResourceItem&), 
		void (ResourcesPanel::*onDoubleClick)(ResourceItem&), 
		void (ResourcesPanel::*onRightClick)(ResourceItem&), 
		void (ResourcesPanel::*onRender)(ResourceItem&))
	:
	m_id(id),
	m_name(name),
	m_uuid(uuid),
	m_thumbnailTextureId(thumbnailTextureId),
	OnClick(onClick),
	OnDoubleClick(onDoubleClick),
	OnRightClick(onRightClick),
	OnRender(onRender)
{}

ResourcesPanel::ResourceItem::ResourceItem(ResourceItem&& other) noexcept
	:
	m_id(other.m_id),
	m_name(other.m_name),
	m_uuid(other.m_uuid),
	m_thumbnailTextureId(other.m_thumbnailTextureId),
	OnClick(other.OnClick),
	OnDoubleClick(other.OnDoubleClick),
	OnRightClick(other.OnRightClick),
	OnRender(other.OnRender)
{}

ResourcesPanel::ResourceItem& ResourcesPanel::ResourceItem::operator=(ResourceItem&& other) noexcept
{
	if (&other == this)
	{
		return *this;
	}
	m_id = other.m_id;
	m_name = other.m_name;
	m_uuid = other.m_uuid;
	m_thumbnailTextureId = other.m_thumbnailTextureId;
	OnClick = other.OnClick;
	OnDoubleClick = other.OnDoubleClick;
	OnRightClick = other.OnRightClick;
	OnRender = other.OnRender;
	return *this;
}

// End Resources Item

// Resources Panel
ResourcesPanel::ResourcesPanel()
	:
	m_isOpened(true),
	m_currentPath(ProgramContext::Get().GetProjectAssetsDirectoryPath()),
	m_pathToDisplay(m_currentPath.filename().string()),
	m_jumpsFromRoot(0),
	m_directoryIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "DirectoryIcon.png")),
	m_sceneIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "SceneIcon.png")),
	m_materialIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "MaterialIcon.png")),
	m_animationIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "AnimationIcon.png")),
	m_animationStateMachineIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "AnimationStateMachineIcon.png")),
	m_physicsMaterialIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "PhysicsMaterialIcon.png")),
	m_cppFileIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "CppFileIcon.png")),
	m_headerFileIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "HeaderFileIcon.png")),
	m_spriteSheetGenIcon(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "SpriteSheetGenIcon.png")),
	m_cellSize(64),
	m_resourceBeingRenamed(nullptr)
{
	ProbeCurrentDirectory();
	Window::Get().AddDragAndDropCallback([&](size_t count, const char** paths) -> void { WindowDragAndDropCallback(count, paths); });
}

void ResourcesPanel::Render()
{
	static bool isResourceHovered(true);
	if (!m_isOpened)
	{
		return;
	}
	ImGuiWindowFlags windowFlags(0);
	if (!ImGui::Begin("Resources", &m_isOpened, windowFlags))
	{
		ImGui::End();
		return;
	}
	if (!isResourceHovered && ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("Popup");
	}
	if (ImGui::BeginPopup("Popup"))
	{
		if (ImGui::Button("Import Resource"))
		{
			static std::filesystem::path resourcePath;
			if (FileBrowser::Get().BrowseFile("Select resource", resourcePath))
			{
				EditorAssetManager::Get().ImportResource(resourcePath, m_currentPath);
				ProbeCurrentDirectory();
			}
		}
		if (ImGui::Button("Create"))
		{
			ImGui::OpenPopup("Create Popup");
		}
		if (ImGui::BeginPopup("Create Popup"))
		{
			if (ImGui::Button("Scene"))
			{
				ImGui::OpenPopup("Create Scene Popup");
			}
			if (ImGui::BeginPopup("Create Scene Popup"))
			{
				MakeEnterNamePopup("Enter scene name:", "Scene names cannot be empty!", &ResourcesPanel::CreateScene);
				ImGui::EndPopup();
			}
			if (ImGui::Button("Material"))
			{
				ImGui::OpenPopup("Create Material Popup");
			}
			if (ImGui::BeginPopup("Create Material Popup"))
			{
				if (ImGui::Button("Unlit Sprite Material"))
				{
					ImGui::OpenPopup("Create Sprite Unlit Material Popup");
				}
				if (ImGui::BeginPopup("Create Sprite Unlit Material Popup"))
				{
					MakeEnterNamePopup("Enter material name:", "A material name cannot be empty!", &ResourcesPanel::CreateUnlitSpriteMaterial);
					ImGui::EndPopup();
				}
				if (ImGui::Button("Lit Sprite Material"))
				{
					ImGui::OpenPopup("Create Lit Sprite Material Popup");
				}
				if (ImGui::BeginPopup("Create Lit Sprite Material Popup"))
				{
					// TODO: Create lit sprite material.
					ImGui::EndPopup();
				}
				if (ImGui::Button("Physics Material"))
				{
					ImGui::OpenPopup("Create Physics Material Popup");
				}
				if (ImGui::BeginPopup("Create Physics Material Popup"))
				{
					MakeEnterNamePopup("Enter material name:", "A material name cannot be empty", &ResourcesPanel::CreatePhysicsMaterial);
					ImGui::EndPopup();
				}
				ImGui::EndPopup();
			}
			if (ImGui::Button("Animaton"))
			{
				ImGui::OpenPopup("CreateAnimationPopup");
			}
			if (ImGui::BeginPopup("CreateAnimationPopup"))
			{
				MakeEnterNamePopup("Enter animation name:", "Animation names cannot be empty", &ResourcesPanel::CreateAnimation);
				ImGui::EndPopup();
			}
			if (ImGui::Button("Animation State Machine"))
			{
				ImGui::OpenPopup("CreateAnimationStateMachinePopup");
			}
			if (ImGui::BeginPopup("CreateAnimationStateMachinePopup"))
			{
				MakeEnterNamePopup("Enter animation state machine name", "Name cannot be empty", &ResourcesPanel::CreateAnimationStateMachine);
				ImGui::EndPopup();
			}
			if (ImGui::Button("Script Component"))
			{
				ImGui::OpenPopup("CreateScriptComponentPopup");
			}
			if (ImGui::BeginPopup("CreateScriptComponentPopup"))
			{
				MakeEnterNamePopup("Enter script name", "Name cannot be empty", &ResourcesPanel::CreateScriptComponent);
				ImGui::EndPopup();
			}
			if (ImGui::Button("Directory"))
			{
				ImGui::OpenPopup("CreateDirectoryPopup");
			}
			if (ImGui::BeginPopup("CreateDirectoryPopup"))
			{
				MakeEnterNamePopup("Enter directory name", "Name cannot be empty", &ResourcesPanel::CreateDirectory);
				ImGui::EndPopup();
			}
			if (ImGui::Button("Sprite Sheet Generator"))
			{
				ImGui::OpenPopup("CreateSpriteSheetGeneratorPopup");
			}
			if (ImGui::BeginPopup("CreateSpriteSheetGeneratorPopup"))
			{
				MakeEnterNamePopup("Enter sprite sheet generator name", "Name cannot be empty", &ResourcesPanel::CreateSpriteSheetGen);
				ImGui::EndPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}
	if (m_jumpsFromRoot > 0 && ImGui::ArrowButton("Back button", ImGuiDir_Left))
	{
		m_jumpsFromRoot--;
		m_currentPath = m_currentPath.parent_path();
		m_pathToDisplay.Clear();
		m_pathToDisplay = m_currentPath.filename().string();
		ProbeCurrentDirectory();
	}
	ImGui::SameLine();
	ImGui::Text("%s", m_pathToDisplay.Data());
	ImGui::Separator();
	float cellSizePlusPadding(1.3f * m_cellSize);
	size_t numberOfColumns(ImGui::GetContentRegionAvail().x / cellSizePlusPadding);
	if (numberOfColumns < 1)
	{
		numberOfColumns = 1;
	}
	bool resourceHovered(false);
	if (ImGui::BeginTable("Resources Table", numberOfColumns))
	{
		size_t columnIndex(0);
		for (size_t resourceIndex(0); resourceIndex < m_resourceItems.size(); resourceIndex++)
		{
			if (resourceIndex % numberOfColumns == 0)
			{
				ImGui::TableNextRow();
			}
			if (columnIndex == numberOfColumns)
			{
				columnIndex = 0;
			}
			ImGui::TableSetColumnIndex(columnIndex);
			if (ImGui::BeginTable("Resource Table", 1))
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ResourceItem& resourceItem(m_resourceItems[resourceIndex]);
				(*this.*resourceItem.OnRender)(resourceItem);
				if (ImGui::BeginPopupContextItem())
				{
					(*this.*resourceItem.OnRightClick)(resourceItem);
					ImGui::EndPopup();
				}
				if (ImGui::IsItemHovered() && !ImGui::IsDragDropActive())
				{
					resourceHovered = true;
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						(*this.*resourceItem.OnDoubleClick)(resourceItem);
					}
					else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						(*this.*resourceItem.OnClick)(resourceItem);
					}
				}
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::TextWrapped("%s", resourceItem.GetName().Data());
				ImGui::EndTable();
			}
			columnIndex++;
		}
		ImGui::EndTable();
	}
	if (resourceHovered)
	{
		isResourceHovered = true;
	}
	else
	{
		isResourceHovered = false;
	}
	ImGui::End();
}

void ResourcesPanel::ProbeCurrentDirectory()
{
	constexpr DCore::DUInt directoryId(0);
	constexpr DCore::DUInt sceneId(1);
	constexpr DCore::DUInt texture2DId(2);
	constexpr DCore::DUInt spriteMaterialId(3);
	constexpr DCore::DUInt animationId(4);
	constexpr DCore::DUInt animationStateMachineId(5);
	constexpr DCore::DUInt physicsMaterialId(6);
	constexpr DCore::DUInt spriteSheetGenId(7);
	constexpr DCore::DUInt cppFileId(8);
	constexpr DCore::DUInt headerFileId(9);
	m_resourceItems.clear();
	for (const auto& dirEntry : std::filesystem::directory_iterator(m_currentPath))
	{
		if (dirEntry.is_directory())
		{
			std::filesystem::path dirName(dirEntry.path().filename());
			if (dirName == "scene" || 
				dirName == "texture" || 
				dirName == "material" || 
				dirName == "animation" || 
				dirName == "animation state machine" || 
				dirName == "physics material" ||
				dirName == "bank" ||
				dirName == "sprite sheet gen")
			{
				continue;
			}
			m_resourceItems.push_back
			(
				ResourceItem
				(
					directoryId, 
					dirEntry.path().stem().string(), 
					m_directoryIcon.GetId(), 
					&ResourcesPanel::OnDirectoryClick, 
					&ResourcesPanel::OnDirectoryDoubleClick, 
					&ResourcesPanel::OnDirectoryRightClick, 
					&ResourcesPanel::OnRender
				)
			);
			continue;
		}
		const DCore::DString extension(dirEntry.path().filename().extension().string());
		if (extension == ".dtscene")
		{
			YAML::Node sceneNode(YAML::LoadFile(dirEntry.path().string()));
			DASSERT_E(sceneNode["UUID"]);
			const DCore::UUIDType sceneUUID(sceneNode["UUID"].as<std::string>());
			m_resourceItems.push_back
			(
				ResourceItem
				(
					sceneId, 
					dirEntry.path().stem().string(), 
					sceneUUID,
					m_sceneIcon.GetId(), 
					&ResourcesPanel::OnSceneClick, 
					&ResourcesPanel::OnSceneDoubleClick, 
					&ResourcesPanel::OnSceneRightClick, 
					&ResourcesPanel::OnRenderScene
				)
			);
			continue;
		}
		if (extension == ".dttex")
		{
			YAML::Node texNode(YAML::LoadFile(dirEntry.path().string()));
			DASSERT_E(texNode["UUID"]);
			const DCore::UUIDType uuid(texNode["UUID"].as<std::string>());
			m_resourceItems.push_back
			(
				ResourceItem
				(
					texture2DId,
					dirEntry.path().filename().stem().string(),
					uuid,
					TextureManager::Get().LoadTexture2D(uuid).GetId(),		// Texture 2D "leak".
					&ResourcesPanel::OnTexture2DClick,
					&ResourcesPanel::OnTexture2DDoubleClick,
					&ResourcesPanel::OnTexture2DRightClick,
					&ResourcesPanel::OnRenderTexture2D
				)
			);
			continue;
		}
		if (extension == ".dtsprmat")
		{
			YAML::Node spriteMaterialNode(YAML::LoadFile(dirEntry.path().string()));
			DASSERT_E(spriteMaterialNode["UUID"]);
			const DCore::UUIDType uuid(spriteMaterialNode["UUID"].as<std::string>());
			m_resourceItems.push_back
			(
				ResourceItem
				(
					spriteMaterialId,
					dirEntry.path().filename().stem().string(),
					uuid,
					m_materialIcon.GetId(),
					&ResourcesPanel::OnSpriteMaterialClick,
					&ResourcesPanel::OnSpriteMaterialDoubleClick,
					&ResourcesPanel::OnSpriteMaterialRightClick,
					&ResourcesPanel::OnRenderSpriteMaterial
				)
			);
			continue;
		}
		if (extension == ".dtanim")
		{
			YAML::Node animationNode(YAML::LoadFile(dirEntry.path().string()));
			DASSERT_E(animationNode["UUID"]);
			const DCore::UUIDType uuid(animationNode["UUID"].as<std::string>());
			m_resourceItems.push_back
			(
				ResourceItem
				(
					animationId,
					dirEntry.path().filename().stem().string(),
					uuid,
					m_animationIcon.GetId(),
					&ResourcesPanel::OnAnimationClick,
					&ResourcesPanel::OnAnimationDoubleClick,
					&ResourcesPanel::OnAnimationRightClick,
					&ResourcesPanel::OnRenderAnimation
				)
			);
			continue;
		}
		if (extension == ".dtasm")
		{
			YAML::Node animationStateMachineNode(YAML::LoadFile(dirEntry.path().string()));
			DASSERT_E(animationStateMachineNode["UUID"]);
			const DCore::UUIDType uuid(animationStateMachineNode["UUID"].as<std::string>());
			m_resourceItems.push_back
			(
				ResourceItem
				(
					animationStateMachineId,
					dirEntry.path().filename().stem().string(),
					uuid,
					m_animationStateMachineIcon.GetId(),
					&ResourcesPanel::OnAnimationStateMachineClick,
					&ResourcesPanel::OnAnimationStateMachineDoubleClick,
					&ResourcesPanel::OnAnimationStateMachineRightClick,
					&ResourcesPanel::OnRenderAnimationStateMachine
				)
			);
			continue;
		}
		if (extension == ".dtphysmat")
		{
			YAML::Node physicsMatrialNode(YAML::LoadFile(dirEntry.path().string()));
			DASSERT_E(physicsMatrialNode["UUID"]);
			const DCore::UUIDType uuid(physicsMatrialNode["UUID"].as<stringType>());
			m_resourceItems.push_back
			(
				ResourceItem
				(
					physicsMaterialId,
					dirEntry.path().filename().stem().string(),
					uuid,
					m_physicsMaterialIcon.GetId(),
					&ResourcesPanel::OnPhysicsMaterialClick,
					&ResourcesPanel::OnPhysicsMaterialDoubleClick,
					&ResourcesPanel::OnPhysicsMaterialRightClick,
					&ResourcesPanel::OnRenderPhysicsMaterial
				)
			);
			continue;
		}
		if (extension == SpriteSheetGenManager::spriteSheetGenThumbnailExtension)
		{
			YAML::Node node(YAML::LoadFile(dirEntry.path().string()));
			DASSERT_E(node["UUID"]);
			const DCore::UUIDType uuid(node["UUID"].as<stringType>());
			m_resourceItems.push_back
			(
				ResourceItem
				(
					spriteSheetGenId,
					dirEntry.path().filename().stem().string(),
					uuid,
					m_spriteSheetGenIcon.GetId(),
					&ResourcesPanel::OnSpriteSheetGenClick,
					&ResourcesPanel::OnSpriteSheetGenDoubleClick,
					&ResourcesPanel::OnSpriteSheetGenRightClick,
					&ResourcesPanel::OnRenderSpriteSheetGen
				)
			);
			continue;
		}
		if (extension == ".cpp")
		{
			m_resourceItems.push_back
			(
				ResourceItem
				(
					cppFileId,
					dirEntry.path().filename().stem().string(),
					m_cppFileIcon.GetId(),
					&ResourcesPanel::OnCppFileClick,
					&ResourcesPanel::OnCppFileDoubleClick,
					&ResourcesPanel::OnCppFileRightClick,
					&ResourcesPanel::OnRenderCppFile
				)
			);
			continue;
		}
		if (extension == ".h")
		{
			m_resourceItems.push_back
			(
				ResourceItem
				(
					headerFileId,
					dirEntry.path().filename().stem().string(),
					m_headerFileIcon.GetId(),
					&ResourcesPanel::OnHeaderFileClick,
					&ResourcesPanel::OnHeaderFileDoubleClick,
					&ResourcesPanel::OnHeaderFileRightClick,
					&ResourcesPanel::OnRenderHeaderFile
				)
			);
			continue;
		}
	}
	std::sort(m_resourceItems.begin(), m_resourceItems.end(), ResourceItemComparator());
}

void ResourcesPanel::WindowDragAndDropCallback(size_t count, const char** paths)
{
	for (size_t i(0); i < count; i++)
	{
		EditorAssetManager::Get().ImportResource(paths[i], m_currentPath);
	}
	ProbeCurrentDirectory();
}

void ResourcesPanel::OnDirectoryClick(ResourceItem& resourceItem)
{}

void ResourcesPanel::OnSceneClick(ResourceItem&)
{}

void ResourcesPanel::OnTexture2DClick(ResourceItem&)
{}

void ResourcesPanel::OnSpriteMaterialClick(ResourceItem&)
{}

void ResourcesPanel::OnAnimationClick(ResourceItem&)
{}

void ResourcesPanel::OnAnimationStateMachineClick(ResourceItem&)
{}

void ResourcesPanel::OnPhysicsMaterialClick(ResourceItem&)
{}

void ResourcesPanel::OnCppFileClick(ResourceItem&)
{}

void ResourcesPanel::OnHeaderFileClick(ResourceItem&)
{}

void ResourcesPanel::OnSpriteSheetGenClick(ResourceItem&)
{}

void ResourcesPanel::OnDirectoryDoubleClick(ResourceItem& resourceItem)
{
	const DCore::DString& directoryName(resourceItem.GetName());
	m_currentPath /= directoryName.Data();
	m_pathToDisplay.Append("/");
	m_pathToDisplay.Append(m_currentPath.filename().string().c_str());
	ProbeCurrentDirectory();
	m_jumpsFromRoot++;
}

void ResourcesPanel::OnSceneDoubleClick(ResourceItem& resourceItem)
{
	if (GameStatePanel::Get().GetGameState() == GameState::Playing)
	{
		Log::Get().TerminalLog("%s", "Cannot load a scene from the editor while the game is playing.");
		Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot load a scene from the editor while the game is playing.");
		return;
	}
	std::filesystem::path sceneThumbnailPath(m_currentPath / resourceItem.GetName().Data());
	sceneThumbnailPath += ".dtscene";
	YAML::Node sceneNode(YAML::LoadFile(sceneThumbnailPath.string()));
	DASSERT_E(sceneNode);
	std::filesystem::path scenePath(ProgramContext::Get().GetProjectAssetsDirectoryPath());
	DASSERT_E(sceneNode["UUID"]);
	const DCore::UUIDType uuid(sceneNode["UUID"].as<std::string>());
	DCore::SceneRef scene;
	SceneManager::Get().LoadScene(uuid, &scene);
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	if (scene.IsValid())
	{
		scene.LoadingCompleted();
	}
}

void ResourcesPanel::OnTexture2DDoubleClick(ResourceItem& resourceItem)
{
	if (TexturePanel::IsPanelWithTextureUUIDOpened(resourceItem.GetUUID()))
	{
		Log::Get().TerminalLog("A panel for the texture with name %s is already opened", resourceItem.GetName().Data());
		Log::Get().ConsoleLog(LogLevel::Error, "A panel for the texture with name %s is already opened", resourceItem.GetName().Data());
		return;
	}
	TexturePanel::OpenTexturePanel(resourceItem.GetName().Data(), TextureManager::Get().LoadTexture2D(resourceItem.GetUUID()));
}
 
void ResourcesPanel::OnSpriteMaterialDoubleClick(ResourceItem& resourceItem)
{
	if (SpriteMaterialPanel::IsPanelWithSpriteMaterialUUIDOpened(resourceItem.GetUUID()))
	{
		Log::Get().TerminalLog("A panel for the sprite material with name %s is already opened", resourceItem.GetName().Data());
		Log::Get().ConsoleLog(LogLevel::Error, "A panel for the sprite material with name %s is already opened", resourceItem.GetName().Data());
		return;
	}
	SpriteMaterialPanel::OpenSpriteMaterialPanel(resourceItem.GetName().Data(), MaterialManager::Get().LoadSpriteMaterial(resourceItem.GetUUID()));
}

void ResourcesPanel::OnAnimationDoubleClick(ResourceItem& resourceItem)
{
	if (!AnimationPanel::IsPanelWithAnimationUUIDOpened(resourceItem.GetUUID()))
	{
		AnimationPanel::OpenAnimationPanel(resourceItem.GetName().Data(), AnimationManager::Get().LoadAnimation(resourceItem.GetUUID()));
		return;
	}
	Log::Get().ConsoleLog(LogLevel::Error, "Cannot open animation panel for animation \"%s\": panel is already opened.", resourceItem.GetName().Data());
}

void ResourcesPanel::OnAnimationStateMachineDoubleClick(ResourceItem& resourceItem)
{
	using parameterInfoContainerType = AnimationStateMachinePanel::parameterInfoContainerType;
	if (AnimationStateMachinePanel::IsAnimationStateMachinePanelOpened(resourceItem.GetUUID()))
	{
		Log::Get().ConsoleLog(LogLevel::Error, "Animation state machine panel for animation state machine \"%s\" is already opened.", resourceItem.GetName().Data());
		return;
	}
	parameterInfoContainerType parameterInfos;
	EditorAnimationStateMachine editorAnimationStateMachine(AnimationStateMachineManager::Get().LoadAnimationStateMachine(resourceItem.GetUUID(), static_cast<stringType>(resourceItem.GetName().Data()), &parameterInfos));
	AnimationStateMachinePanel::OpenAnimationStateMachinePanel(resourceItem.GetName().Data(), std::move(editorAnimationStateMachine), std::move(parameterInfos));
}

void ResourcesPanel::OnPhysicsMaterialDoubleClick(ResourceItem& resourceItem)
{
	using physicsMaterialRefType = DCore::PhysicsMaterialRef;
	if (PhysicsMaterialPanel::IsPanelWithPhyiscsMaterialUUIDOpened(resourceItem.GetUUID()))
	{
		Log::Get().ConsoleLog(LogLevel::Error, "Physics material panel for physics material \"%s\" is already opened.", resourceItem.GetName().Data());
		return;
	}
	physicsMaterialRefType physicsMaterial(PhysicsMaterialManager::Get().LoadPhysicsMaterial(resourceItem.GetUUID()));
	PhysicsMaterialPanel::OpenPhysicsMaterialPanel(resourceItem.GetName().Data(), physicsMaterial);
}

void ResourcesPanel::OnCppFileDoubleClick(ResourceItem&)
{}

void ResourcesPanel::OnHeaderFileDoubleClick(ResourceItem&)
{}

void ResourcesPanel::OnSpriteSheetGenDoubleClick(ResourceItem& resourceItem)
{
	if (SpriteSheetGenManager::Get().IsSpriteSheetGenLoaded(resourceItem.GetUUID()))
	{
		Log::Get().TerminalLog("Sprite sheet generator panel for sprite sheet generator \"%s\" is already opened.", resourceItem.GetName().Data());
		Log::Get().ConsoleLog(LogLevel::Error, "Sprite sheet generator panel for sprite sheet generator \"%s\" is already opened.", resourceItem.GetName().Data());
		return;	
	}
	SpriteSheetGenManager::spriteSheetGenRefType spriteSheetGen(SpriteSheetGenManager::Get().LoadSpriteSheetGen(resourceItem.GetUUID()));
	if (spriteSheetGen.IsValid())
	{
		SpriteSheetGenPanel::OpenSpriteSheetGenPanel(spriteSheetGen);
	}
}

void ResourcesPanel::OnDirectoryRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenameDirectoryPopup");
	}
	if (ImGui::BeginPopup("RenameDirectoryPopup"))
	{
		MakeEnterNamePopup("Enter new director name", "Direcory names cannot be empty", &ResourcesPanel::RenameDirectory);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnSceneRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Delete"))
	{
		if (GameStatePanel::Get().GetGameState() == GameState::Playing)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot delete assets while the game is playing.");
			return;
		}
		SceneManager::Get().DeleteScene(resourceItem.GetUUID());
		const std::filesystem::path sceneThumbnailPath((m_currentPath / resourceItem.GetName().Data()).concat(".dtscene"));
		std::filesystem::remove(sceneThumbnailPath);
		ProbeCurrentDirectory();
	}
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenameScenePopup");
	}
	if (ImGui::BeginPopup("RenameScenePopup"))
	{
		MakeEnterNamePopup("Enter scene new name", "Scenes names cannot be empty", &ResourcesPanel::RenameScene);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnTexture2DRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Delete"))
	{
		if (GameStatePanel::Get().GetGameState() == GameState::Playing)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot delete assets while the game is playing.");
			return;
		}
		TexturePanel::ClosePanelWithTextureUUID(resourceItem.GetUUID());
		TextureManager::Get().DeleteTexture(resourceItem.GetUUID());
		std::filesystem::path textureThumbnailPath(m_currentPath / resourceItem.GetName().Data());
		textureThumbnailPath += ".dttex";
		std::filesystem::remove(textureThumbnailPath);
		ProbeCurrentDirectory();
	}
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenameTexturePopup");
	}
	if (ImGui::BeginPopup("RenameTexturePopup"))
	{
		MakeEnterNamePopup("Enter texture name", "Textures names cannot be empty", &ResourcesPanel::RenameTexture2D);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnSpriteMaterialRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Delete"))
	{
		if (GameStatePanel::Get().GetGameState() == GameState::Playing)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot delete assets while the game is playing.");
			return;
		}
		MaterialManager::Get().DeleteSpriteMaterial(resourceItem.GetUUID());
		std::filesystem::path spriteMaterialThumbnailPath(m_currentPath / resourceItem.GetName().Data());
		spriteMaterialThumbnailPath += ".dtsprmat";
		std::filesystem::remove(spriteMaterialThumbnailPath);
		ProbeCurrentDirectory();
	}
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenameSpriteMaterialPopup");
	}
	if (ImGui::BeginPopup("RenameSpriteMaterialPopup"))
	{
		MakeEnterNamePopup("Enter sprite material name", "Sprite materials names cannot be empty", &ResourcesPanel::RenameSpriteMaterial);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnAnimationRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Delete"))
	{
		if (GameStatePanel::Get().GetGameState() == GameState::Playing)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot delete assets while the game is playing.");
			return;
		}
		AnimationManager::Get().DeleteAnimation(resourceItem.GetUUID());
		std::filesystem::path animationThumbnailPath(m_currentPath / resourceItem.GetName().Data());
		animationThumbnailPath += ".dtanim";
		std::filesystem::remove(animationThumbnailPath);
		ProbeCurrentDirectory();
	}
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenameAnimationPopup");
	}
	if (ImGui::BeginPopup("RenameAnimationPopup"))
	{
		MakeEnterNamePopup("Enter animation name", "Animations names cannot be empty", &ResourcesPanel::RenameAnimation);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnAnimationStateMachineRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Delete"))
	{
		if (GameStatePanel::Get().GetGameState() == GameState::Playing)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot delete assets while the game is playing.");
			return;
		}
		AnimationStateMachineManager::Get().DeleteAnimationStateMachine(resourceItem.GetUUID());
		std::filesystem::path asmThumbnailPath(m_currentPath / resourceItem.GetName().Data());
		asmThumbnailPath += ".dtasm";
		std::filesystem::remove(asmThumbnailPath);
		AnimationStateMachinePanel::CloseAnimationStateMachinePanelWithUUID(resourceItem.GetUUID());
		ProbeCurrentDirectory();
	}
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenameAnimationStateMachinePopup");
	}
	if (ImGui::BeginPopup("RenameAnimationStateMachinePopup"))
	{
		MakeEnterNamePopup("Enter animation state machine name", "Animation state machines names cannot be empty", &ResourcesPanel::RenameAnimationStateMachine);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnPhysicsMaterialRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Delete"))
	{
		if (GameStatePanel::Get().GetGameState() == GameState::Playing)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot delete assets while the game is playing.");
			return;
		}
		PhysicsMaterialManager::Get().DeletePhysicsMaterial(resourceItem.GetUUID());
		std::filesystem::path physicsMaterialThumbnailPath(m_currentPath / resourceItem.GetName().Data());
		physicsMaterialThumbnailPath += ".dtphysmat";
		std::filesystem::remove(physicsMaterialThumbnailPath);
		ProbeCurrentDirectory();
	}
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenamePhysicsMaterialPopup");
	}
	if (ImGui::BeginPopup("RenamePhysicsMaterialPopup"))
	{
		MakeEnterNamePopup("Enter physics material name", "Physics materials names cannot be empty", &ResourcesPanel::RenamePhysicsMaterial);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnCppFileRightClick(ResourceItem&)
{}

void ResourcesPanel::OnHeaderFileRightClick(ResourceItem&)
{}

void ResourcesPanel::OnSpriteSheetGenRightClick(ResourceItem& resourceItem)
{
	if (ImGui::Selectable("Delete"))
	{
		SpriteSheetGenManager::Get().DeleteSpriteSheetGen(resourceItem.GetUUID());
		std::filesystem::remove(m_currentPath / (DCore::DString(resourceItem.GetName()).Append(SpriteSheetGenManager::spriteSheetGenThumbnailExtension)).Data());
		ProbeCurrentDirectory();
	}
	if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
	{
		m_resourceBeingRenamed = &resourceItem;
		ImGui::OpenPopup("RenameSpriteSheetGenPopup");
	}
	if (ImGui::BeginPopup("RenameSpriteSheetGenPopup"))
	{
		MakeEnterNamePopup("Enter sprite sheet generator name", "Sprite sheet generators names cannot be empty", &ResourcesPanel::RenameSpriteSheetGen);
		ImGui::EndPopup();
	}
}

void ResourcesPanel::OnRender(ResourceItem& resourceItem)
{
	ImGui::ImageButton(resourceItem.GetName().Data(), (ImTextureID)(intptr_t)resourceItem.GetThumbnailTextureId(), ImVec2(m_cellSize, m_cellSize), {0, 1}, {1, 0});
}

void ResourcesPanel::OnRenderScene(ResourceItem& resourceItem)
{
	OnRender(resourceItem);
	if (!ImGui::BeginDragDropSource())
	{
		return;
	}
	std::filesystem::path sceneThumbnailPath(m_currentPath / resourceItem.GetName().Data());
	sceneThumbnailPath += ".dtscene";
	YAML::Node sceneNode(YAML::LoadFile(sceneThumbnailPath.string()));
	DASSERT_E(sceneNode["UUID"]);
	const DCore::UUIDType uuid(sceneNode["UUID"].as<std::string>());
	ImGui::SetDragDropPayload("SCENE_UUID", &uuid, sizeof(DCore::UUIDType));
	ImGui::Text("%s", resourceItem.GetName().Data());
	ImGui::EndDragDropSource();
}

void ResourcesPanel::OnRenderTexture2D(ResourceItem& resourceItem)
{
	OnRender(resourceItem);
	if (!ImGui::BeginDragDropSource())
	{
		return;
	}
	std::filesystem::path textureThumbnailPath(m_currentPath / resourceItem.GetName().Data());
	textureThumbnailPath += ".dttex";
	YAML::Node texture2DNode(YAML::LoadFile(textureThumbnailPath.string()));
	DASSERT_E(texture2DNode["UUID"]);
	const DCore::UUIDType uuid(texture2DNode["UUID"].as<std::string>());
	DCore::Texture2DRef texture2DRef(TextureManager::Get().LoadTexture2D(uuid));
	ImGui::SetDragDropPayload("TEXTURE_2D_REF", &texture2DRef, sizeof(DCore::Texture2DRef));
	ImGui::Text("%s", resourceItem.GetName().Data());
	ImGui::EndDragDropSource();
}

void ResourcesPanel::OnRenderSpriteMaterial(ResourceItem& resourceItem)
{
	OnRender(resourceItem);
	if (!ImGui::BeginDragDropSource())
	{
		return;
	}
	ImGui::SetDragDropPayload("SPRITE_MATERIAL_UUID", &resourceItem.GetUUID(), sizeof(DCore::UUIDType));
	ImGui::Text("%s", resourceItem.GetName().Data());
	ImGui::EndDragDropSource();
}

void ResourcesPanel::OnRenderAnimation(ResourceItem& resourceItem)
{
	OnRender(resourceItem);
	if (!ImGui::BeginDragDropSource())
	{
		return;
	}
	AnimationPayload animationPayload(resourceItem.GetUUID());
	ImGui::SetDragDropPayload("ANIMATION_PAYLOAD", &animationPayload, sizeof(AnimationPayload));
	ImGui::Text("%s", resourceItem.GetName().Data());
	ImGui::EndDragDropSource();
}

void ResourcesPanel::OnRenderAnimationStateMachine(ResourceItem& resourceItem)
{
	OnRender(resourceItem);
	if (!ImGui::BeginDragDropSource())
	{
		return;
	}
	AnimationStateMachinePayload payload{resourceItem.GetUUID(), resourceItem.GetName()};
	ImGui::SetDragDropPayload("ANIMATION_STATE_MACHINE_PAYLOAD", &payload, sizeof(AnimationStateMachinePayload));
	ImGui::Text("%s", resourceItem.GetName().Data());
	ImGui::EndDragDropSource();
}

void ResourcesPanel::OnRenderPhysicsMaterial(ResourceItem& resourceItem)
{
	OnRender(resourceItem);
	if (!ImGui::BeginDragDropSource())
	{
		return;
	}
	PhysicsMaterialPayload payload{resourceItem.GetUUID()};
	ImGui::SetDragDropPayload("PHYSICS_MATERIAL_PAYLOAD", &payload, sizeof(PhysicsMaterialPayload));
	ImGui::Text("%s", resourceItem.GetName().Data());
	ImGui::EndDragDropSource();
}

void ResourcesPanel::OnRenderCppFile(ResourceItem& resourceItem)
{
	OnRender(resourceItem);	
}

void ResourcesPanel::OnRenderSpriteSheetGen(ResourceItem& resourceItem)
{
	OnRender(resourceItem);
}

void ResourcesPanel::OnRenderHeaderFile(ResourceItem& resourceItem)
{
	OnRender(resourceItem);	
}

void ResourcesPanel::MakeEnterNamePopup(const char* inputMessage, const char* emptyNameMessage, void (ResourcesPanel::*callback)(const stringType&))
{
	static char name[32];
	static bool toCallback(false);
	static bool toCancel(false);
	static bool emptyName(false);
	ImGui::Text("%s", inputMessage);
	ImGui::PushID("Enter Name Input Text");
	if (ImGui::InputText("", name, STRING_SIZE, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		toCallback = true;
	}
	ImGui::PopID();
	if (emptyName)
	{
		ImGui::Text("%s", emptyNameMessage);
	}
	if (toCallback)
	{
		toCallback = false;
		if (std::strlen(name) == 0)
		{
			emptyName = true;
		}
		else
		{
			(*this.*callback)(name);
			ProbeCurrentDirectory();
			ImGui::CloseCurrentPopup();
		}
	}
	else if (toCancel)
	{
		ImGui::CloseCurrentPopup();
		toCancel = false;
	}
	toCallback = ImGui::Button("Create");
	ImGui::SameLine();
	toCancel = ImGui::Button("Cancel");
}

void ResourcesPanel::CreateDirectory(const stringType& name)
{
	if (name == "animation" ||
		name == "animation state machine" ||
		name == "bank" ||
		name == "material" ||
		name == "phyiscs material" ||
		name == "scene" ||
		name == "texture")
	{
		Log::Get().TerminalLog("The directory name %s is reserved", name.c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "The directory name %s is reserved", name.c_str());
		return;
	}
	const std::filesystem::path newDirectoryPath(m_currentPath / name);
	std::filesystem::create_directory(newDirectoryPath);
	std::filesystem::permissions(newDirectoryPath, std::filesystem::perms::all);
}

void ResourcesPanel::CreateScene(const stringType& sceneName)
{
	SceneManager::Get().CreateScene(sceneName, m_currentPath);
}

void ResourcesPanel::CreateUnlitSpriteMaterial(const stringType& materialName)
{
	MaterialManager::Get().CreateSpriteMaterial(DCore::SpriteMaterialType::Unlit, materialName, m_currentPath);
}

void ResourcesPanel::CreateAnimation(const stringType& animationName)
{
	AnimationManager::Get().CreateAnimation(animationName, m_currentPath);
}

void ResourcesPanel::CreateAnimationStateMachine(const stringType& name)
{
	AnimationStateMachineManager::Get().CreateAnimationStateMachine(name, m_currentPath);
}

void ResourcesPanel::CreatePhysicsMaterial(const stringType& name)
{
	PhysicsMaterialManager::Get().CreatePhysicsMaterial(name, m_currentPath);
}

void ResourcesPanel::CreateScriptComponent(const stringType& name)
{
	// TODO. Create a script component in path m_currentPath.
	std::filesystem::path scriptPath(m_currentPath / name.c_str());
	scriptPath += ".h";
	std::ifstream ifstream(scriptPath);
	if (ifstream)
	{
		Log::Get().TerminalLog("Script component with name %s is already created", name.c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Script component with name %s is already created", name.c_str());
		ifstream.close();
		return;
	}
	ifstream.close();
	std::ofstream ofstream(scriptPath);
	DASSERT_E(ofstream);
	char script[4096];
	// TODO. Move to the heap
	const char* scriptTemplate = R"(#pragma once

#include "DommusCore.h"



namespace Game
{
	class %s;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::%s>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class %s : public DCore::ScriptComponent
{
public:
	%s(const DCore::ConstructorArgs<%s>&);
	~%s() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override
	{}

	virtual void Update(float deltaTime) override
	{}

	virtual void LateUpdate(float deltaTime) override
	{}

	virtual void PhysicsUpdate(float physicsDeltaTime) override
	{}

	virtual void PhysicsLateUpdate(float physicsDeltaTime) override
	{}
};

class %sScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~%sScriptComponentFormGenerator() = default;
private:
	%sScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"%s",
			DCore::ComponentId::GetId<%s>(),
			sizeof(%s),
			sizeof(DCore::ConstructorArgs<%s>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) %s(*static_cast<const DCore::ConstructorArgs<%s>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<%s*>(componentAddress)->~%s();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<%s> m_defaultArgs;
private:
	static %sScriptComponentFormGenerator s_generator;
};

}
	)";
	std::sprintf(script, scriptTemplate, 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(), 
			name.c_str(),
			name.c_str(),
			name.c_str(),
			name.c_str(),
			name.c_str(),
			name.c_str(),
			name.c_str(),
			name.c_str(),
			name.c_str(),
			name.c_str());	
	ofstream << script;
	DASSERT_E(ofstream);
	ofstream.close();
	DASSERT_E(ofstream);
}

void ResourcesPanel::CreateSpriteSheetGen(const stringType& name)
{
	SpriteSheetGenManager::Get().CreateSpriteSheetGen(name, m_currentPath);
}

void ResourcesPanel::RenameDirectory(const stringType& name)
{
	// TODO. Update the paths of all assets inside this directory.
	DASSERT_E(m_resourceBeingRenamed != nullptr);
	const std::filesystem::path newName(m_currentPath / name);
	std::error_code ec;
	try
	{
		std::filesystem::rename(m_currentPath / m_resourceBeingRenamed->GetName().Data(), newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename directory: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename directory: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	m_resourceBeingRenamed = nullptr;
}

void ResourcesPanel::RenameScene(const stringType& name)
{
	DASSERT_E(m_resourceBeingRenamed != nullptr);
	if (!SceneManager::Get().RenameScene(m_resourceBeingRenamed->GetUUID(), name))
	{
		m_resourceBeingRenamed = nullptr;
		return;
	}
	std::filesystem::path oldName(m_currentPath / m_resourceBeingRenamed->GetName().Data());
	oldName += ".dtscene";
	std::filesystem::path newName(m_currentPath / name);
	newName += ".dtscene";
	std::error_code ec;
	try
	{
		std::filesystem::rename(oldName, newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename scene: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename scene: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	m_resourceBeingRenamed = nullptr;
}

void ResourcesPanel::RenameTexture2D(const stringType& name)
{
	DASSERT_E(m_resourceBeingRenamed != nullptr);
	if (!TextureManager::Get().RenameTexture(m_resourceBeingRenamed->GetUUID(), name))
	{
		m_resourceBeingRenamed = nullptr;
		return;
	}
	std::filesystem::path oldName(m_currentPath / m_resourceBeingRenamed->GetName().Data());
	oldName += ".dttex";
	std::filesystem::path newName(m_currentPath / name);
	newName += ".dttex";
	std::error_code ec;
	try
	{
		std::filesystem::rename(oldName, newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename texture: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename texture: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	m_resourceBeingRenamed = nullptr;
}

void ResourcesPanel::RenameSpriteMaterial(const stringType& name)
{
	if (!MaterialManager::Get().RenameSpriteMaterial(m_resourceBeingRenamed->GetUUID(), name))
	{
		m_resourceBeingRenamed = nullptr;
		return;
	}
	std::filesystem::path oldName(m_currentPath / m_resourceBeingRenamed->GetName().Data());
	oldName += ".dtsprmat";
	std::filesystem::path newName(m_currentPath / name);
	newName += ".dtsprmat";
	std::error_code ec;
	try
	{
		std::filesystem::rename(oldName, newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename sprite material: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename sprite material: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	// TODO. Consider renaming opened panels.
	m_resourceBeingRenamed = nullptr;
}

void ResourcesPanel::RenameAnimation(const stringType& name)
{
	if (!AnimationManager::Get().RenameAnimation(m_resourceBeingRenamed->GetUUID(), name))
	{
		m_resourceBeingRenamed = nullptr;
		return;
	}
	std::filesystem::path oldName(m_currentPath / m_resourceBeingRenamed->GetName().Data());
	oldName += ".dtanim";
	std::filesystem::path newName(m_currentPath / name);
	newName += ".dtanim";
	std::error_code ec;
	try
	{
		std::filesystem::rename(oldName, newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename animation: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename animation: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	// TODO. Consider renaming opened panels.
	m_resourceBeingRenamed = nullptr;
}

void ResourcesPanel::RenameAnimationStateMachine(const stringType& name)
{
	if (!AnimationStateMachineManager::Get().RenameAnimationStateMachine(m_resourceBeingRenamed->GetUUID(), name))
	{
		m_resourceBeingRenamed = nullptr;
		return;
	}
	std::filesystem::path oldName(m_currentPath / m_resourceBeingRenamed->GetName().Data());
	oldName += ".dtasm";
	std::filesystem::path newName(m_currentPath / name);
	newName += ".dtasm";
	std::error_code ec;
	try
	{
		std::filesystem::rename(oldName, newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename animation state machine: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename animation state machine: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	// TODO. Consider renaming opened panels.
	m_resourceBeingRenamed = nullptr;
}

void ResourcesPanel::RenamePhysicsMaterial(const stringType& name)
{
	if (!PhysicsMaterialManager::Get().RenamePhysicsMaterial(m_resourceBeingRenamed->GetUUID(), name))
	{
		m_resourceBeingRenamed = nullptr;
		return;
	}
	std::filesystem::path oldName(m_currentPath / m_resourceBeingRenamed->GetName().Data());
	oldName += ".dtphysmat";
	std::filesystem::path newName(m_currentPath / name);
	newName += ".dtphysmat";
	std::error_code ec;
	try
	{
		std::filesystem::rename(oldName, newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename physics material: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename physics material: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	// TODO. Consider renaming opened panels.
	m_resourceBeingRenamed = nullptr;
}

void ResourcesPanel::RenameSpriteSheetGen(const stringType& name)
{
	if (!SpriteSheetGenManager::Get().RenameSpriteSheetGen(m_resourceBeingRenamed->GetUUID(), name))
	{
		m_resourceBeingRenamed = nullptr;
		return;
	}
	const std::filesystem::path oldName(m_currentPath / (DCore::DString(m_resourceBeingRenamed->GetName()).Append(SpriteSheetGenManager::spriteSheetGenThumbnailExtension).Data()));
	const std::filesystem::path newName(m_currentPath / (name + SpriteSheetGenManager::spriteSheetGenThumbnailExtension));
	std::error_code ec;
	try
	{
		std::filesystem::rename(oldName, newName, ec);
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Fail to rename sprite sheet generator: %s\n\tError code: %s", e.what(), ec.message().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to rename sprite sheet generator: %s\n\tError code: %s", e.what(), ec.message().c_str());
	}
	m_resourceBeingRenamed = nullptr;
}
// End Resources Panel

}
