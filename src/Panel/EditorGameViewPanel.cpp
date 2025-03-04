#include "EditorGameViewPanel.h"
#include "ConsolePanel.h"
#include "ProgramContext.h"
#include "Log.h"
#include "SceneHierarchyPanel.h"
#include "EditorGameViewPanels.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuizmo.h"

#include <array>
#include <cstdint>



namespace DEditor
{

EditorGameViewPanel::EditorGameViewPanel(const char* panelName, size_t panelId, bool isOpened)
	:
	m_isOpened(isOpened),
	m_panelName(panelName),
	m_panelId(panelId),
	m_renderingDone(false),
	m_cameraComponent(DCore::ConstructorArgs<DCore::PerspectiveCameraComponent>()),
	m_cameraTransformComponent(DCore::ConstructorArgs<DCore::TransformComponent>()),
	m_cameraVelocity(30.0f),
	m_deltaCameraVelocity(10.0f),
	m_isWindowHovered(false),
	m_isGizmoActive(false),
	m_gizmoOperation(GizmoOperation::Translation),
	m_gizmoSpace(GizmoSpace::Global)
{
	if (m_isOpened)
	{
		m_isGizmoActive = true;
		m_renderer.Initiate(ProgramContext::Get().GetMainContext());
	}
}

EditorGameViewPanel::~EditorGameViewPanel()
{
	if (m_isOpened)
	{
		m_renderer.Terminate();
	}
}

void EditorGameViewPanel::Open()
{
	m_isOpened = true;
	m_renderer.Initiate(ProgramContext::Get().GetMainContext());
}

void EditorGameViewPanel::Update()
{
	if (!m_isOpened)
	{
		m_renderer.Terminate();
		return;
	}
	if (!ImGui::Begin(m_panelName.c_str(), &m_isOpened))
	{
		ImGui::End();
		return;
	}
	m_viewportSizes = ImGui::GetContentRegionAvail();
	if (m_viewportSizes.x <= 0 || m_viewportSizes.y <= 0)
	{
		ImGui::End();
		return;
	}
	// Camera Controll
	m_isWindowHovered = ImGui::IsWindowHovered();
	if (m_isWindowHovered && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		ImGuiIO& io(ImGui::GetIO());
		bool toMoveInHorizontal(false);
		bool toMoveInVertical(false);
		bool toMoveInScreenDirection(false);
		bool toMoveRight(false);
		bool toMoveUp(false);
		bool toMoveOutside(false);
		if (ImGui::IsKeyDown(ImGuiKey_A))
		{
			toMoveInHorizontal = true;
		}
		else if (ImGui::IsKeyDown(ImGuiKey_D))
		{
			toMoveInHorizontal = true;
			toMoveRight = true;
		}
		if (ImGui::IsKeyDown(ImGuiKey_W))
		{
			toMoveInVertical = true;
			toMoveUp = true;
		}
		else if (ImGui::IsKeyDown(ImGuiKey_S))
		{
			toMoveInVertical = true;
		}
		if (ImGui::IsKeyDown(ImGuiKey_E))
		{
			toMoveInScreenDirection = true;
		}
		else if (ImGui::IsKeyDown(ImGuiKey_Q))
		{
			toMoveInScreenDirection = true;
			toMoveOutside = true;
		}
		DCore::DVec3 translation(m_cameraVelocity * io.DeltaTime, m_cameraVelocity * io.DeltaTime, m_cameraVelocity * io.DeltaTime);
		if (!toMoveInHorizontal)
		{
			translation.x = 0;
		}
		else
		{
			if (!toMoveRight)
			{
				translation.x *= -1;
			}
		}
		if (!toMoveInVertical)
		{
			translation.y = 0;
		}
		else
		{
			if (!toMoveUp)
			{
				translation.y *= -1;
			}
		}
		if (!toMoveInScreenDirection)
		{
			translation.z = 0;
		}
		else
		{
			if (!toMoveOutside)
			{
				translation.z *= -1;
			}
		}
		if (translation != DCore::DVec3(0.0f, 0.0f, 0.0f))
		{
			m_cameraTransformComponent.AddTranslation(translation);
		}
		m_cameraVelocity = std::max(m_cameraVelocity + m_deltaCameraVelocity * io.MouseWheel, 2.5f);
		
	}
	// End camera Controll
	//
	ImVec2 cursorPos(ImGui::GetCursorScreenPos());
	DCore::DVec2 mouseLocalPos(GetMouseLocalPosition());
	bool isUsingGizmo(false);
	unsigned int outputTextureId(m_renderer.GetOutputTextureId());
	ImGui::Image((ImTextureID)(uintptr_t)outputTextureId, m_viewportSizes, {0, 1}, {1, 0});
	// Gizmo Draw
	if (SceneHierarchyPanel::Get().IsEntitySelected() && m_isGizmoActive)
	{
		DCore::EntityRef entityRef(SceneHierarchyPanel::Get().GetSelectedEntityRef());
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
		DCore::ComponentRef<DCore::TransformComponent> transformComponent(entityRef.GetComponents<DCore::TransformComponent>());
		DASSERT_E(transformComponent.IsValid());
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(cursorPos.x, cursorPos.y, m_viewportSizes.x, m_viewportSizes.y);
		DCore::DMat4 modelMatrix;
		if (entityRef.HaveParent())
		{
			modelMatrix = entityRef.GetWorldModelMatrix();
		}
		else
		{
			modelMatrix = transformComponent.GetModelMatrix();
		}
		ImGuizmo::OPERATION operation;
		switch (m_gizmoOperation)
		{
		case GizmoOperation::Translation:
			operation = ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y;
			break;
		case GizmoOperation::Rotation:
			operation = ImGuizmo::ROTATE_Z;
			break;
		case GizmoOperation::Scale:
			operation = ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y;
		}
		ImGuizmo::Manipulate
		(
			glm::value_ptr(m_cameraTransformComponent.GetInverseModelMatrix()), 
			glm::value_ptr(m_cameraComponent.GetProjectionMatrix({m_viewportSizes.x, m_viewportSizes.y})),
			operation,
			m_gizmoSpace == GizmoSpace::Local ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
			glm::value_ptr(modelMatrix)
		);
		if (ImGuizmo::IsUsing())
		{
			DCore::DVec2 translation, scale;
			DCore::DFloat rotation;
			entityRef.RemoveParentTransformationsFrom(modelMatrix);
			DCore::Math::Decompose(modelMatrix, translation, rotation, scale);
			transformComponent.SetTranslation({translation.x, translation.y, transformComponent.GetTranslation().z}); 
			transformComponent.SetRotation(rotation);
			transformComponent.SetScale(scale);
			isUsingGizmo = true;
		}
	}
	if (!isUsingGizmo)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_T))
		{
			m_gizmoOperation = GizmoOperation::Translation;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_R))
		{
			m_gizmoOperation = GizmoOperation::Rotation;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_Y))
		{
			m_gizmoOperation = GizmoOperation::Scale;
		}
		if (ImGui::IsKeyPressed(ImGuiKey_G))
		{
			m_gizmoSpace = GizmoSpace::Global;
		}	
		else if (ImGui::IsKeyPressed(ImGuiKey_L))
		{
			m_gizmoSpace = GizmoSpace::Local;
		}
	}
	// End Gizmo Draw
	// Entity Selection
	if (m_isWindowHovered && !isUsingGizmo && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		mouseLocalPos.y = m_viewportSizes.y - mouseLocalPos.y;
		if (std::array<int, 4> pixelData; m_renderer.TryReadPixelFromClickingTexture(mouseLocalPos, pixelData))
		{
			if (pixelData[0] == -1 || (pixelData[0] == 0 && pixelData[1] == 0 && pixelData[2] == 0 && pixelData[3] == 0))
			{
				SceneHierarchyPanel::Get().ClearEntitySelection();
			}
			else
			{
				DCore::SceneRef sceneRef(DCore::AssetManager::Get().GetSceneRefFromSceneRefId(pixelData[2]));
				DCore::EntityRef entityRef(sceneRef.GetEntityWithIdAndVersion(pixelData[0], pixelData[1]), sceneRef);
				DASSERT_E(entityRef.IsValid());
				SceneHierarchyPanel::Get().SetSelectedEntityRef(entityRef);
				m_isGizmoActive = true;
				EditorGameViewPanels::Get().OnGizmoActivation(m_panelId);
			}
		}
	}
	// End Entity Selecion.
	ImGui::End();
}

bool EditorGameViewPanel::IsRenderingDone()
{
	return m_renderer.IsRenderingDone();
}

void EditorGameViewPanel::Render()
{
	if (m_viewportSizes.x <= 0 || m_viewportSizes.y <= 0)
	{
		return;
	}
	DASSERT_E(m_isOpened && m_renderer.IsRenderingDone());
	MakeViewProjection(m_viewportSizes);
	m_renderer.Begin(DCore::DVec2(m_viewportSizes.x, m_viewportSizes.y));
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	DCore::Runtime::MakeRendererSubmitions(m_viewProjection, m_renderer);
	m_renderer.Render();
}

void EditorGameViewPanel::MakeViewProjection(ImVec2 viewportSizes)
{
	m_viewProjection = m_cameraComponent.GetProjectionMatrix({viewportSizes.x, viewportSizes.y}) * m_cameraTransformComponent.GetInverseModelMatrix();
}

}
