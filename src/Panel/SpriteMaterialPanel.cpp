#include "SpriteMaterialPanel.h"
#include "TextureManager.h"
#include "ProgramContext.h"
#include "MaterialManager.h"

#include "imgui.h"



namespace DEditor
{

SpriteMaterialPanel::spriteMaterialPanelContainerType SpriteMaterialPanel::s_spriteMaterialPanels;

SpriteMaterialPanel::SpriteMaterialPanel(const stringType& panelName, spriteMaterialRefType spriteMaterial)
	:
	m_panelName(panelName),
	m_spriteMaterial(spriteMaterial),
	m_isOpened(true),
	m_windowFlags(0),
	m_whiteMap(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "WhiteMap.png"))
{}

SpriteMaterialPanel::SpriteMaterialPanel(SpriteMaterialPanel&& other) noexcept
	:
	m_panelName(std::move(other.m_panelName)),
	m_spriteMaterial(std::move(other.m_spriteMaterial)),
	m_isOpened(other.m_isOpened),
	m_windowFlags(other.m_windowFlags),
	m_whiteMap(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "WhiteMap.png"))
{}

void SpriteMaterialPanel::OpenSpriteMaterialPanel(const stringType& panelName, spriteMaterialRefType spriteMaterial)
{
	s_spriteMaterialPanels.PushBack(panelName, spriteMaterial);
}

bool SpriteMaterialPanel::IsPanelWithSpriteMaterialUUIDOpened(const uuidType& uuid)
{
	bool returnValue(false);
	s_spriteMaterialPanels.Iterate
	(
		[&](decltype(s_spriteMaterialPanels)::ConstRef spriteMaterialPanel) -> bool
		{
			if (spriteMaterialPanel->GetSpriteMaterialUUID() == uuid)
			{
				returnValue = true;
				return true;
			}
			return false;
		}
	);
	return returnValue;
}

void SpriteMaterialPanel::RenderPanels()
{
	s_spriteMaterialPanels.Iterate
	(
		[&](decltype(s_spriteMaterialPanels)::Ref spriteMaterialPanel) -> bool
		{
			if (spriteMaterialPanel->Render())
			{
				spriteMaterialPanel->UnloadSpriteMaterial();
				s_spriteMaterialPanels.Remove(spriteMaterialPanel);
			}
			return false;
		}
	);
}

bool SpriteMaterialPanel::Render()
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
	if (ImGui::Button("Save"))
	{
		m_windowFlags = 0;
		MaterialManager::Get().SaveSpriteMaterial(m_spriteMaterial);
	}
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
	switch (m_spriteMaterial.GetType())
	{
	case DCore::SpriteMaterialType::Unlit:
		if (ImGui::TreeNodeEx("Diffuse", ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::Selectable("Clear"))
				{
					m_spriteMaterial.ClearDiffuseMapRef();
					m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
					DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
					OnMaterialChanged();
				}
				ImGui::EndPopup();
			}
			DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
			if (m_spriteMaterial.GetDiffuseMapRef().IsValid())
			{
				DrawImageButton(m_spriteMaterial.GetDiffuseMapRef().GetId(), DCore::DVec2(64, 64), DCore::DVec4(1, 1, 1, 1));
			}
			else
			{
				DrawImageButton(m_whiteMap.GetId(), DCore::DVec2(64, 64), DCore::DVec4(0, 0, 0, 1));
			}
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload(ImGui::AcceptDragDropPayload("TEXTURE_2D_REF"));
				if (payload != nullptr)
				{
					DCore::Texture2DRef texture2DRef(*(DCore::Texture2DRef*)payload->Data);
					m_spriteMaterial.SetDiffuseMapRef(texture2DRef);
					m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
					OnMaterialChanged();
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			dVec4 newDiffuseColor(m_spriteMaterial.GetDiffuseColor());
			if (DrawColorPicker(newDiffuseColor))
			{
				m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
				m_spriteMaterial.SetDiffuseColor(newDiffuseColor);
			}
			ImGui::TreePop();
		}
		break;
	case DCore::SpriteMaterialType::Lit:
		break;
	default:
		break;
	}
	ImGui::End();
	return false;
}

SpriteMaterialPanel::uuidType SpriteMaterialPanel::GetSpriteMaterialUUID() const
{
	return m_spriteMaterial.GetUUID();
}

void SpriteMaterialPanel::UnloadSpriteMaterial()
{
	DCore::ReadWriteLockGuard spriteMaterialGuard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
	DCore::ReadWriteLockGuard textureGuard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
	m_spriteMaterial.Unload();
}

bool SpriteMaterialPanel::DrawImageButton(unsigned int textureId, const dVec2& size, const dVec4& tintColor)
{
	return ImGui::ImageButton("##Image Button", (ImTextureID)(intptr_t)textureId, ImVec2(size.x, size.y), {0, 1}, {1, 0}, {1, 1, 1, 1}, {tintColor.r, tintColor.g, tintColor.b, tintColor.a});
}

bool SpriteMaterialPanel::DrawColorPicker(dVec4& outColor)
{
	return ImGui::ColorEdit4("##ColorPicker", &outColor.x, ImGuiColorEditFlags_NoInputs);
}

void SpriteMaterialPanel::OnMaterialChanged()
{
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes
	(
		[&](DCore::SceneRef scene) -> bool
		{
			scene.Iterate<DCore::SpriteComponent>
			(
				[&](DCore::Entity entity, DCore::ComponentRef<DCore::SpriteComponent> spriteComponent) -> bool
				{
					if (DCore::SpriteMaterialRef spriteMaterialRef(spriteComponent.GetSpriteMaterialRef()); spriteMaterialRef.IsValid() && spriteMaterialRef == m_spriteMaterial)
					{
						spriteComponent.FillSpriteQuads();
					}
					return false;
				}
			);
			return false;
		}
	);
}

}
