#pragma once

#include "Panel.h"

#include "DommusCore.h"

#include <string>



namespace DEditor
{

class TexturePanel : public Panel
{
public:
	using textureRefType = DCore::Texture2DRef;		
	using texturePanelContainerType = DCore::ReciclingVector<TexturePanel>;
	using uuidType = DCore::UUIDType;
	using stringType = std::string;
public:
	TexturePanel(const stringType& textureName, textureRefType texture);
	TexturePanel(TexturePanel&&);
	~TexturePanel() = default;
public:
	static bool IsPanelWithTextureUUIDOpened(const uuidType&);
	static void OpenTexturePanel(const stringType& textureName, textureRefType texture);
	static void ClosePanelWithTextureUUID(const uuidType&);
	static void RenderTexturePanels();
public:
	bool Render();
	void Close();
public:
	const uuidType GetTextureUUID() const
	{
		return m_texture.GetUUID();
	}
private:
	static texturePanelContainerType s_texturePanels;
private:
	stringType m_textureName;
	textureRefType m_texture;	
	bool m_isOpened;
	int m_windowFlags;
};

}
