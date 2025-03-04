#include "TexturePanel.h"
#include "TextureManager.h"

#include "imgui.h"



namespace DEditor
{

TexturePanel::texturePanelContainerType TexturePanel::s_texturePanels;

TexturePanel::TexturePanel(const stringType& textureName, textureRefType texture)
	:
	m_textureName(textureName),
	m_texture(texture),
	m_isOpened(true),
	m_windowFlags(0)
{}

TexturePanel::TexturePanel(TexturePanel&& other)
	:
	m_textureName(std::move(other.m_textureName)),
	m_texture(other.m_texture),
	m_isOpened(other.m_isOpened),
	m_windowFlags(other.m_windowFlags)
{}

bool TexturePanel::IsPanelWithTextureUUIDOpened(const uuidType& uuid)
{
	bool returnValue(false);
	s_texturePanels.Iterate
	(
		[&](texturePanelContainerType::ConstRef texturePanel) -> bool
		{
			if (texturePanel->GetTextureUUID() == uuid)
			{
				returnValue = true;
				return true;
			}
			return false;
		}
	);
	return returnValue;
}

void TexturePanel::OpenTexturePanel(const stringType& textureName, textureRefType texture)
{
	s_texturePanels.PushBack(textureName, texture);
}

void TexturePanel::ClosePanelWithTextureUUID(const uuidType& uuid)
{
	s_texturePanels.Iterate
	(
		[&](texturePanelContainerType::Ref texturePanel) -> bool
		{
			if (texturePanel->GetTextureUUID() == uuid)
			{
				texturePanel->Close();
				s_texturePanels.Remove(texturePanel);
			}
			return false;
		}
	);
}

void TexturePanel::RenderTexturePanels()
{
	s_texturePanels.Iterate
	(
		[&](texturePanelContainerType::Ref texturePanel) -> bool
		{
			if (texturePanel->Render())
			{
				texturePanel->Close();
				s_texturePanels.Remove(texturePanel);
			}
			return false;
		}
	);
}

bool TexturePanel::Render()
{
	if (!ImGui::Begin(m_textureName.c_str(), &m_isOpened, m_windowFlags))
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
		TextureManager::Get().SaveTexture(m_texture);
	}
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Filter");
	ImGui::SameLine();
	constexpr size_t numberOfFilters(3);
	static const char* filters[numberOfFilters]{"Nearest", "Bilinear", "Trilinear"};
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
	if (ImGui::BeginCombo("##Filter", filters[static_cast<size_t>(m_texture.GetFilter())]))
	{
		for (size_t i(0); i < numberOfFilters; i++)
		{
			if (ImGui::Selectable(filters[i]))
			{
				m_texture.SetFilter(static_cast<DCore::Texture2DFilter>(i));
				m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::End();
	return false;
}

void TexturePanel::Close()
{
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
	if (m_texture.IsValid())
	{
		m_texture.Unload();
	}
}

}
