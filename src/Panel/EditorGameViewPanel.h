#pragma once

#include "Panel.h"

#include "DommusCore.h"


#include "imgui.h"

#include <cstdint>
#include <string>



namespace DEditor
{

class EditorGameViewPanel : public Panel
{
public:
	using stringType = std::string;
public:
	EditorGameViewPanel(const char* panelName, size_t panelId, bool isOpened = false);
	~EditorGameViewPanel();
private:
	enum class GizmoOperation
	{
		Translation,
		Rotation,
		Scale
	};
	enum class GizmoSpace
	{
		Local,
		Global
	};
public:
	void Open();
	void Update();
	bool IsRenderingDone();
	void Render();
//void SetTextureSlotPack(DCore::TextureSlotPack);
public:
	bool IsOpened()
	{
		return m_isOpened;
	}

	void DisableGizmo()
	{
		m_isGizmoActive = false;
	}

	void SetPanelName(const stringType& name)
	{
		m_panelName = name;
	}

	void SetPanelId(size_t panelId)
	{
		m_panelId = panelId;
	}
private:
	bool m_isOpened;
	stringType m_panelName;
	size_t m_panelId;
	bool m_renderingDone;
	DCore::Renderer m_renderer;
	DCore::PerspectiveCameraComponent m_cameraComponent;
	DCore::TransformComponent m_cameraTransformComponent;
	DCore::DMat4 m_viewProjection;
	ImVec2 m_viewportSizes;
	float m_cameraVelocity;
	float m_deltaCameraVelocity;
	bool m_isWindowHovered;
	bool m_isGizmoActive;
	GizmoOperation m_gizmoOperation;
	GizmoSpace m_gizmoSpace;
private:
	void MakeViewProjection(ImVec2 viewportSizes);
};

}
