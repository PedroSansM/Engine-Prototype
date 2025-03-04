#pragma once

#include "InspectorPanel.h"

#include "DommusCore.h"



namespace DEditor
{

class SceneHierarchyPanel 
{
	friend class Panels;
	friend class UtilityPanel;
public:
	~SceneHierarchyPanel() = default;
public:
	static SceneHierarchyPanel& Get()
	{
		static SceneHierarchyPanel sceneHierarchyPanel;
		return sceneHierarchyPanel;
	}
public:
	bool IsEntitySelected()
	{
		return m_selectedEntityRef.IsValid(); 
	}

	DCore::EntityRef GetSelectedEntityRef()
	{
		DASSERT_E(m_selectedEntityRef.IsValid());
		return m_selectedEntityRef;
	}

	void SetSelectedEntityRef(DCore::EntityRef entity)
	{
		m_selectedEntityRef = entity;
		InspectorPanel::Get().DisplayEntity(m_selectedEntityRef);
	}

	void ClearEntitySelection()
	{
		InspectorPanel::Get().Clear();
		m_selectedEntityRef.Invalidate();
	}
private:
	SceneHierarchyPanel();
private:
	bool m_isOpened;
	DCore::EntityRef m_selectedEntityRef;
private:
	void Open();
	void Render();
	void IterateOnScene(DCore::SceneRef);	
	bool IterateOnEntity(DCore::EntityRef);
	bool MakeEntityNamePopup(
			const char* popupName, 
			bool (SceneHierarchyPanel::*callback)(const char*, DCore::EntityRef), 
			DCore::EntityRef entityRef, 
			const char* fmt, ...);
	bool RenameEntityCallback(const char* newEntityName, DCore::EntityRef);
	bool AddChildEntityCallback(const char* entityName, DCore::EntityRef);
	bool CreateEntityCallback(const char* entityName, DCore::EntityRef);
};

}
