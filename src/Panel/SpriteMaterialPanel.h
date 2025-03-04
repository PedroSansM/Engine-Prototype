#pragma once

#include "Panel.h"

#include "DommusCore.h"

#include <string>



namespace DEditor
{

class SpriteMaterialPanel : public Panel
{
public:
	using texture2DType = DCore::Texture2D;
	using uuidType = DCore::UUIDType;
	using spriteMaterialRefType = DCore::SpriteMaterialRef;
	using spriteMaterialPanelContainerType = DCore::ReciclingVector<SpriteMaterialPanel>;
	using dVec2 = DCore::DVec2;
	using dVec4 = DCore::DVec4;
	using stringType = std::string;
public:
	SpriteMaterialPanel(const stringType& panelName, spriteMaterialRefType);
	SpriteMaterialPanel(SpriteMaterialPanel&&) noexcept;
	~SpriteMaterialPanel() = default;
public:
	static void OpenSpriteMaterialPanel(const stringType& panelName, spriteMaterialRefType);
	static bool IsPanelWithSpriteMaterialUUIDOpened(const uuidType&);
	static void RenderPanels();
public:
	/// Returns if the panel have to be closed.
	bool Render();
	uuidType GetSpriteMaterialUUID() const;
	void UnloadSpriteMaterial();
private:
	static spriteMaterialPanelContainerType s_spriteMaterialPanels;
private:
	stringType m_panelName;
	spriteMaterialRefType m_spriteMaterial;
	bool m_isOpened;
	int m_windowFlags;
	texture2DType m_whiteMap;
private:
	bool DrawImageButton(unsigned int textureId, const dVec2& size, const dVec4& tintColor);														// Its not unique by default.
	bool DrawColorPicker(dVec4& outColor);																											// Its not unique by default.			

	// For now, consider only diffuse map changes. Needs SpriteMaterialAssetManager and Texture2DAssetManager read lock.
	void OnMaterialChanged();
};

}
