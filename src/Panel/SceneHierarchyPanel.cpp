#include "SceneHierarchyPanel.h"
#include "GameViewPanel.h"
#include "InspectorPanel.h"

#include "imgui.h"

#include <cstdarg>
#include <cstring>
#include <iostream>



namespace DEditor
{

SceneHierarchyPanel::SceneHierarchyPanel()
	:
	m_isOpened(true)
{}

void SceneHierarchyPanel::Open()
{
	if (m_isOpened)
	{
		return;
	}
	m_isOpened = true;
	m_selectedEntityRef.Invalidate();
}

void SceneHierarchyPanel::Render()
{
	if (!m_isOpened)
	{
		return;
	}
	ImGuiWindowFlags windowFlags(0);
	if (!ImGui::Begin("Scene Hierarchy", &m_isOpened, windowFlags))
	{
		ImGui::End();
		return;
	}
	if (ImGui::IsKeyPressed(ImGuiKey_Delete) && ImGui::IsWindowFocused() && m_selectedEntityRef.IsValid())
	{
		GameViewPanel::Get().GetRuntime().DestroyEntity(m_selectedEntityRef);
		m_selectedEntityRef.Destroy();
		m_selectedEntityRef.Invalidate();
	}
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes([&](DCore::SceneRef sceneRef) -> bool { IterateOnScene(sceneRef); return false; });
	ImGui::End();
}

void SceneHierarchyPanel::IterateOnScene(DCore::SceneRef sceneRef)
{
	DCore::DString sceneName;
	sceneRef.GetName(sceneName);
	bool isSceneHeaderOpened(ImGui::CollapsingHeader(sceneName.Data()));
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Button("Create Entity"))
		{
			ImGui::OpenPopup("Create Entity Popup");
		}
		MakeEntityNamePopup
		(
			"Create Entity Popup",
			 &SceneHierarchyPanel::CreateEntityCallback,
			 DCore::EntityRef(DCore::Entity(), sceneRef),
			"Enter entity name:"
		);
		ImGui::EndPopup();
	}
	if (!isSceneHeaderOpened)
	{
		return;
	}
	if (sceneRef.GetNumberOfEntitiesWithComponents<DCore::RootComponent>() == 0)
	{
		return;
	}
	sceneRef.Iterate<DCore::RootComponent, DCore::NameComponent>
	(
		[&](DCore::Entity entity, DCore::ComponentRef<DCore::RootComponent>, DCore::ComponentRef<DCore::NameComponent> nameComponent) -> bool
		{
			return IterateOnEntity(DCore::EntityRef(entity, sceneRef));
		}
	);
}

bool SceneHierarchyPanel::IterateOnEntity(DCore::EntityRef entityRef)
{
	if (!entityRef.HaveComponents<DCore::UUIDComponent, DCore::NameComponent>())
	{
		return false;
	}
	bool returnValue(false);
	DCore::SceneRef sceneRef(entityRef.GetSceneRef());
	auto [uuidComponent, nameComponent] = entityRef.GetComponents<DCore::UUIDComponent, DCore::NameComponent>();
	size_t numberOfChildren(0);
	if (entityRef.HaveComponents<DCore::ChildrenComponent>())
	{
		numberOfChildren = entityRef.GetComponents<DCore::ChildrenComponent>().GetNumberOfChildren();
	}
	ImGuiTreeNodeFlags flags(ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | (numberOfChildren == 0 ? ImGuiTreeNodeFlags_Leaf : 0) | (m_selectedEntityRef.IsValid() && m_selectedEntityRef == entityRef && m_selectedEntityRef.GetSceneRef() == sceneRef ? ImGuiTreeNodeFlags_Selected : 0));
	DCore::DString entityName;
	DCore::UUIDType entityUUID;
	nameComponent.GetName(entityName);
	uuidComponent.GetUUID(entityUUID);
	bool isNodeOpen(ImGui::TreeNodeEx(((std::string)entityUUID).c_str(), flags, "%s", entityName.Data()));
	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload("ENTITY_REF", &entityRef, sizeof(DCore::EntityRef));
		ImGui::Text("%s", entityName.Data());
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload(ImGui::AcceptDragDropPayload("ENTITY_REF"));
		if (payload != nullptr)
		{
			DCore::EntityRef source(*(DCore::EntityRef*)payload->Data);
			if (source != entityRef)
			{
				source.SetParent(entityRef);
				returnValue = true;
			}
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::IsItemClicked())
	{
		m_selectedEntityRef = entityRef;
		InspectorPanel::Get().DisplayEntity(m_selectedEntityRef);
	}
	if (ImGui::BeginPopupContextItem())
	{
		m_selectedEntityRef = entityRef;
		InspectorPanel::Get().DisplayEntity(m_selectedEntityRef);
		if (ImGui::Button("Rename"))
		{
			ImGui::OpenPopup("Rename Entity Popup");
		}
		if (ImGui::Button("Add Child"))
		{
			ImGui::OpenPopup("Add Child Popup");
		}
		if (entityRef.HaveParent() && ImGui::Button("Free From Parent"))
		{
			entityRef.RemoveParent();
			returnValue = true;
		}
		if (ImGui::Button("Duplicate"))
		{
			entityRef.Duplicate();
			returnValue = true;
		}
		if (MakeEntityNamePopup
		(
			"Rename Entity Popup",
			&SceneHierarchyPanel::RenameEntityCallback,
			entityRef,
			"Enter entity new name:"
		))
		{
			returnValue = true;
		}
		if (MakeEntityNamePopup
		(
			"Add Child Popup",
			&SceneHierarchyPanel::AddChildEntityCallback,
			entityRef,
			"Enter entity name:"
		))
		{
			returnValue = true;
		}
		ImGui::EndPopup();
	}
	if (isNodeOpen)
	{
		if (numberOfChildren > 0 && !returnValue)
		{
			DCore::ComponentRef<DCore::ChildrenComponent> childrenComponent(entityRef.GetComponents<DCore::ChildrenComponent>());
			entityRef.IterateOnChildren
			(
				[&](DCore::EntityRef child) -> bool 
				{ 
					if (IterateOnEntity(child))
					{
						returnValue = true;
						return true;
					}
					return false;
				}
			);
		}
		ImGui::TreePop();
	}
	return returnValue;
}

bool SceneHierarchyPanel::MakeEntityNamePopup(
		const char* popupName, 
		bool (SceneHierarchyPanel::*callback)(const char*, DCore::EntityRef), 
		DCore::EntityRef entityRef, 
		const char* fmt, ...)
{
	bool returnValue(false);
	if (ImGui::BeginPopup(popupName))
	{
		static char newEntityName[STRING_SIZE] = "";
		static bool isNameEmpty(false);
		va_list args;
		va_start(args, fmt);
		ImGui::Text(fmt, args);
		va_end(args);
		ImGui::PushID("Entity Rename");
		ImGuiInputTextFlags inputTextFlags(ImGuiInputTextFlags_EnterReturnsTrue);
		bool enterPressedOnInputText(ImGui::InputText("", newEntityName, STRING_SIZE, inputTextFlags));
		ImGui::PopID();
		if (isNameEmpty)
		{
			ImGui::Text("Entity name cannot be empty!");
		}
		if (enterPressedOnInputText || ImGui::Button("Ok"))
		{
			if (std::strlen(newEntityName) == 0)
			{
				isNameEmpty = true;
			}
			else
			{
				isNameEmpty = false;
				returnValue = (this->*callback)(newEntityName, entityRef);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	return returnValue;
}

bool SceneHierarchyPanel::RenameEntityCallback(const char* newEntityName, DCore::EntityRef entityRef)
{
	entityRef.SetName(newEntityName);
	return false;
}	

bool SceneHierarchyPanel::AddChildEntityCallback(const char* entityName, DCore::EntityRef entityRef)
{
	DCore::SceneRef sceneRef(entityRef.GetSceneRef());
	DCore::EntityRef newEntityRef(sceneRef.CreateEntity(entityName), sceneRef);
	newEntityRef.SetParent(entityRef);
	m_selectedEntityRef = newEntityRef;
	InspectorPanel::Get().DisplayEntity(m_selectedEntityRef);
	return true;
}

bool SceneHierarchyPanel::CreateEntityCallback(const char* entityName, DCore::EntityRef entityRef)
{
	DCore::SceneRef sceneRef(entityRef.GetSceneRef());
	DCore::EntityRef newEntity(sceneRef.CreateEntity(entityName), sceneRef);
	m_selectedEntityRef = newEntity;
	InspectorPanel::Get().DisplayEntity(m_selectedEntityRef);
	return true;
}

}
