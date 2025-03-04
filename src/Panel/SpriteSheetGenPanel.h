#pragma once

#include "SpriteSheetGenManager.h"

#include "DommusCore.h"

#include <unordered_set>
#include <string>
#include <vector>



namespace DEditor
{

class SpriteSheetGenPanel
{
private:
	struct TextureName;
	struct TextureNameComparator;
public:
	using spriteSheetGenRefType = SpriteSheetGenManager::spriteSheetGenRefType;
	using spriteSheetGenPanelContainerType = DCore::ReciclingVector<SpriteSheetGenPanel>;
	using uuidType = DCore::UUIDType;
	using rendererType = DCore::Renderer;
	using perspectiveCameraComponentType = DCore::PerspectiveCameraComponent;
	using transformComponentType = DCore::TransformComponent;
	using entityType = DCore::Entity;
	using dVec2 = DCore::DVec2;
	using dVec4 = DCore::DVec4;
	using dUInt = DCore::DUInt;
	using dMat4 = DCore::DMat4;
	using registryType = DCore::Registry;
	using spriteMaterialRefType = DCore::SpriteMaterialRef;
	using addedTextureUUIDContainerType = std::unordered_set<uuidType>;
	using stringType = std::string;
	using textureNameContainerType = std::vector<TextureName>;
public:
	SpriteSheetGenPanel(spriteSheetGenRefType);
	SpriteSheetGenPanel(const SpriteSheetGenPanel&) = delete;
	SpriteSheetGenPanel(SpriteSheetGenPanel&&) noexcept;
	~SpriteSheetGenPanel();
public:
	static void OpenSpriteSheetGenPanel(spriteSheetGenRefType);
	static void RenderPanels();
public:
	bool Render();
private:
	static spriteSheetGenPanelContainerType s_spriteSheetGenPanels;
	static spriteMaterialRefType s_centerIconSpriteMaterial;
private:
	struct TextureName
	{
		uuidType UUID;
		DCore::DString Name;
	};
private:
	spriteSheetGenRefType m_spriteSheetGen;
	bool m_isOpened;
	int m_windowFlags;
	rendererType m_renderer;
	perspectiveCameraComponentType m_cameraComponent;
	transformComponentType m_cameraTransformComponent;
	addedTextureUUIDContainerType m_addedTextures;
	textureNameContainerType m_textureNames;
	size_t m_selectedSpriteIndex;
	bool m_drawOnionSkin;
	size_t m_onionSkinIndex;
	entityType m_centerIconEntity;
private:
	void DrawConfig();
	void DrawOnionSkin();
	void DrawAddedTextures();
	void DrawCenterIconScale();
	void DrawAddTexture();
	void DrawCanvas();
	void DrawGenerate();
	void SubmitEntity(entityType, registryType&, const dVec2& viewportSizes, const dVec4* overrideTintColor = nullptr, const dUInt* overrideDrawOrder = nullptr, const dMat4* overrideViewporjectionMatrix = nullptr);
};

}
