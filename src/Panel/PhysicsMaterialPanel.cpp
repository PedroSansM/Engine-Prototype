#include "PhysicsMaterialPanel.h"
#include "PhysicsMaterialManager.h"

#include "imgui.h"

#include <cstdio>
#include <float.h>



namespace DEditor
{

PhysicsMaterialPanel::physicsMaterialPanelContainerType PhysicsMaterialPanel::s_physicsMaterialPanels;

PhysicsMaterialPanel::PhysicsMaterialPanel(const stringType& panelName, physicsMaterialRefType physicsMaterial)
	:
	m_panelName(panelName),
	m_physicsMaterial(physicsMaterial),
	m_isOpened(true),
	m_windowFlags(0)
{}

PhysicsMaterialPanel::PhysicsMaterialPanel(PhysicsMaterialPanel&& other) noexcept
	:
	m_panelName(std::move(other.m_panelName)),
	m_physicsMaterial(std::move(other.m_physicsMaterial)),
	m_isOpened(other.m_isOpened),
	m_windowFlags(other.m_windowFlags)
{}

void PhysicsMaterialPanel::OpenPhysicsMaterialPanel(const stringType& panelName, physicsMaterialRefType physicsMaterial)
{
	s_physicsMaterialPanels.PushBack(panelName, physicsMaterial);
}

void PhysicsMaterialPanel::RenderPanels()
{
	s_physicsMaterialPanels.Iterate
	(
		[&](decltype(s_physicsMaterialPanels)::Ref physicsMaterialPanel) -> bool
		{
			if (physicsMaterialPanel->Render())
			{
				physicsMaterialPanel->UnloadPhysicsMaterial();
				s_physicsMaterialPanels.Remove(physicsMaterialPanel);
			}
			return false;
		}
	);
}

bool PhysicsMaterialPanel::IsPanelWithPhysicsMaterialUUIDOpened(const uuidType& uuid)
{
	bool returnValue(false);
	s_physicsMaterialPanels.IterateConstRef
	(
		[&](decltype(s_physicsMaterialPanels)::ConstRef physicsMaterialPanel) -> bool
		{
			if (physicsMaterialPanel->GetPhysicsMaterialUUID() == uuid)
			{
				returnValue = true;
				return true;
			}
			return false;
		}
	);
	return returnValue;
}

bool PhysicsMaterialPanel::Render()
{
	if (!ImGui::Begin(m_panelName.c_str(), &m_isOpened, m_windowFlags))
	{
		ImGui::End();
		return false;
	}
	if (!m_isOpened)
	{
		ImGui::End();
		return true;
	}
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
		if (!m_physicsMaterial.IsValid())
		{
			ImGui::End();
			return true;
		}
	}
	if (ImGui::Button("Save"))
	{
		PhysicsMaterialManager::Get().SaveChanges(m_physicsMaterial);
		m_windowFlags = ImGuiWindowFlags_None;
	}
	DrawDragFloat("Density", &physicsMaterialRefType::GetDensity, &physicsMaterialRefType::SetDensity);
	DrawDragFloat("Friction", &physicsMaterialRefType::GetFriction, &physicsMaterialRefType::SetFriction);
	DrawDragFloat("Restitution", &physicsMaterialRefType::GetRestitution, &physicsMaterialRefType::SetRestitution);
	ImGui::End();
	return false;
}

PhysicsMaterialPanel::uuidType PhysicsMaterialPanel::GetPhysicsMaterialUUID() const
{
	return m_physicsMaterial.GetUUID();
}

void PhysicsMaterialPanel::UnloadPhysicsMaterial()
{
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
	m_physicsMaterial.Unload();
}

void PhysicsMaterialPanel::DrawDragFloat(const char* label, float (physicsMaterialRefType::*get)() const, void (physicsMaterialRefType::*set)(float))
{
	char dragFloatLabel[32];
	std::sprintf(dragFloatLabel, "##%s", label);
	ImGui::Text("%s", label);
	ImGui::SameLine();
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
	float value((m_physicsMaterial.*get)());
	if (ImGui::DragFloat(dragFloatLabel, &value, 0.01f, 0.0f, FLT_MAX))
	{
		m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
		(m_physicsMaterial.*set)(value);
	}
}

}
