#pragma once

#include "Panel.h"
#include "PhysicsMaterial.h"

#include "DommusCore.h"

#include <string>



namespace DEditor
{

class PhysicsMaterialPanel : public Panel
{
public:
	using uuidType = DCore::UUIDType;
	using physicsMaterialRefType = DCore::PhysicsMaterialRef;
	using physicsMaterialPanelContainerType = DCore::ReciclingVector<PhysicsMaterialPanel>;
	using stringType = std::string;
public:
	PhysicsMaterialPanel(const stringType& panelName, physicsMaterialRefType);
	PhysicsMaterialPanel(PhysicsMaterialPanel&&) noexcept;
	~PhysicsMaterialPanel() = default;
public:
	static void OpenPhysicsMaterialPanel(const stringType& panelName, physicsMaterialRefType);
	static bool IsPanelWithPhysicsMaterialUUIDOpened(const uuidType&);
	static void RenderPanels();
public:
	// Returns if the panel have to be closed.
	bool Render();
	uuidType GetPhysicsMaterialUUID() const;
	void UnloadPhysicsMaterial();
private:
	static physicsMaterialPanelContainerType s_physicsMaterialPanels;
private:
	stringType m_panelName;
	physicsMaterialRefType m_physicsMaterial;
	bool m_isOpened;
	int m_windowFlags;
private:
	void DrawDragFloat(const char* label, float (physicsMaterialRefType::*get)() const, void (physicsMaterialRefType::*set)(float));
};

}
