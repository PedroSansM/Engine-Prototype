#pragma once

#include "DommusCore.h"

#include "AnimationPayload.h"

#include <filesystem>
#include <vector>



namespace DEditor
{

class ResourcesPanel
{
	friend class UtilityPanel;
	friend class Panels;
public:
	using stringType = std::string;
public:
	~ResourcesPanel() = default;
private:
	class ResourceItem;
private:
	using ResourceItemComparator = struct ResourceItemComparator
	{
		bool operator()(const ResourceItem& a, const ResourceItem& b) const
		{
			return a.GetId() < b.GetId();
		}
	};
private:
	class ResourceItem
	{
	public:
		ResourceItem(
				DCore::DUInt id, 
				const DCore::DString& name, 
				DCore::DUInt thumbnailTextureId, 
				void (ResourcesPanel::*onClick)(ResourceItem&), 
				void (ResourcesPanel::*onDoubleClick)(ResourceItem&), 
				void (ResourcesPanel::*onRightClick)(ResourceItem&), 
				void (ResourcesPanel::*onRender)(ResourceItem&));
		ResourceItem(
				DCore::DUInt id, 
				const DCore::DString& name, 
				const DCore::UUIDType& uuid, 
				DCore::DUInt thumbnailTextureId, 
				void (ResourcesPanel::*onClick)(ResourceItem&), 
				void (ResourcesPanel::*onDoubleClick)(ResourceItem&), 
				void (ResourcesPanel::*onRightClick)(ResourceItem&),
				void (ResourcesPanel::*onRender)(ResourceItem&));
		ResourceItem(ResourceItem&&) noexcept;
		~ResourceItem() = default;
	public:
		void (ResourcesPanel::*OnClick)(ResourceItem&);
		void (ResourcesPanel::*OnDoubleClick)(ResourceItem&);
		void (ResourcesPanel::*OnRightClick)(ResourceItem&);
		void (ResourcesPanel::*OnRender)(ResourceItem&);
	public:
		DCore::DUInt GetId() const
		{
			return m_id;
		}

		const DCore::DString& GetName() const
		{
			return m_name;
		}

		const DCore::UUIDType& GetUUID() const
		{
			return m_uuid;
		}

		DCore::DUInt GetThumbnailTextureId() const
		{
			return m_thumbnailTextureId;
		}

	public:
		ResourceItem& operator=(ResourceItem&&) noexcept;
	private:
		DCore::DUInt m_id; // Used for sorting.
		DCore::DString m_name;
		DCore::UUIDType m_uuid;
		DCore::DUInt m_thumbnailTextureId;
	};
private:
	static ResourcesPanel& Get()
	{
		static ResourcesPanel resourcesPanel;
		return resourcesPanel;
	}
private:
	void Render();
private:
	void Open()
	{
		m_isOpened = true;
	}
private:
	ResourcesPanel();
private:
	bool m_isOpened;
	std::filesystem::path m_currentPath;
	DCore::DString m_pathToDisplay;
	size_t m_jumpsFromRoot;
	std::vector<ResourceItem> m_resourceItems;
	DCore::Texture2D m_directoryIcon;
	DCore::Texture2D m_sceneIcon;
	DCore::Texture2D m_materialIcon;
	DCore::Texture2D m_animationIcon;
	DCore::Texture2D m_animationStateMachineIcon;
	DCore::Texture2D m_physicsMaterialIcon;
	DCore::Texture2D m_cppFileIcon;
	DCore::Texture2D m_headerFileIcon;
	DCore::Texture2D m_spriteSheetGenIcon;
	size_t m_cellSize;
	ResourceItem* m_resourceBeingRenamed;
private:
	void ProbeCurrentDirectory();
	void WindowDragAndDropCallback(size_t count, const char** paths);
	void OnDirectoryClick(ResourceItem&);	
	void OnSceneClick(ResourceItem&);
	void OnTexture2DClick(ResourceItem&);
	void OnSpriteMaterialClick(ResourceItem&);
	void OnAnimationClick(ResourceItem&);
	void OnAnimationStateMachineClick(ResourceItem&);
	void OnPhysicsMaterialClick(ResourceItem&);
	void OnCppFileClick(ResourceItem&);
	void OnHeaderFileClick(ResourceItem&);
	void OnSpriteSheetGenClick(ResourceItem&);
	void OnDirectoryDoubleClick(ResourceItem&);
	void OnSceneDoubleClick(ResourceItem&);
	void OnTexture2DDoubleClick(ResourceItem&);
	void OnSpriteMaterialDoubleClick(ResourceItem&);
	void OnAnimationDoubleClick(ResourceItem&);
	void OnAnimationStateMachineDoubleClick(ResourceItem&);
	void OnPhysicsMaterialDoubleClick(ResourceItem&);
	void OnCppFileDoubleClick(ResourceItem&);
	void OnHeaderFileDoubleClick(ResourceItem&);
	void OnSpriteSheetGenDoubleClick(ResourceItem&);
	void OnDirectoryRightClick(ResourceItem&);
	void OnSceneRightClick(ResourceItem&);
	void OnTexture2DRightClick(ResourceItem&);
	void OnSpriteMaterialRightClick(ResourceItem&);
	void OnAnimationRightClick(ResourceItem&);
	void OnAnimationStateMachineRightClick(ResourceItem&);
	void OnPhysicsMaterialRightClick(ResourceItem&);
	void OnCppFileRightClick(ResourceItem&);
	void OnHeaderFileRightClick(ResourceItem&);
	void OnSpriteSheetGenRightClick(ResourceItem&);
	void OnRender(ResourceItem&);
	void OnRenderScene(ResourceItem&);
	void OnRenderTexture2D(ResourceItem&);
	void OnRenderSpriteMaterial(ResourceItem&);
	void OnRenderAnimation(ResourceItem&);
	void OnRenderAnimationStateMachine(ResourceItem&);
	void OnRenderPhysicsMaterial(ResourceItem&);
	void OnRenderCppFile(ResourceItem&);
	void OnRenderSpriteSheetGen(ResourceItem&);
	void OnRenderHeaderFile(ResourceItem&);
	void MakeEnterNamePopup(const char* inputMessage, const char* emptyNameMessage, void (ResourcesPanel::*callback)(const stringType&));
	void CreateScene(const stringType& sceneName);
	void CreateDirectory(const stringType& name);
	void CreateUnlitSpriteMaterial(const stringType& materialName);
	void CreateAnimation(const stringType& animationName);
	void CreateAnimationStateMachine(const stringType& name);
	void CreatePhysicsMaterial(const stringType& name);
	void CreateScriptComponent(const stringType& name);
	void CreateSpriteSheetGen(const stringType& name);
	void RenameDirectory(const stringType& name);
	void RenameScene(const stringType& name);
	void RenameTexture2D(const stringType& name);
	void RenameSpriteMaterial(const stringType& name);
	void RenameAnimation(const stringType& name);
	void RenameAnimationStateMachine(const stringType& name);
	void RenamePhysicsMaterial(const stringType& name);
	void RenameSpriteSheetGen(const stringType& name);
};

}
