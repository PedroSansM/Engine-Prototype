#include "SpriteSheetGenPanel.h"
#include "ProgramContext.h"
#include "TextureManager.h"
#include "Log.h"
#include "FileBrowser.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "stb_image.h"
#include "imgui.h"

#include <cfloat>
#include <climits>



namespace DEditor
{

SpriteSheetGenPanel::spriteSheetGenPanelContainerType SpriteSheetGenPanel::s_spriteSheetGenPanels;
SpriteSheetGenPanel::spriteMaterialRefType SpriteSheetGenPanel::s_centerIconSpriteMaterial;

SpriteSheetGenPanel::SpriteSheetGenPanel(spriteSheetGenRefType spriteSheetGen)
	:
	m_spriteSheetGen(spriteSheetGen),
	m_isOpened(true),
	m_windowFlags(ImGuiWindowFlags_None),
	m_cameraComponent(DCore::ConstructorArgs<DCore::PerspectiveCameraComponent>()),
	m_selectedSpriteIndex(0),
	m_cameraTransformComponent({{0.0f, 0.0f, 50.0f}, 0.0f, {1.0f, 1.0f}}),
	m_drawOnionSkin(false),
	m_onionSkinIndex(0)
{
	m_renderer.Initiate(ProgramContext::Get().GetMainContext());
	registryType& registry(m_spriteSheetGen->GetRegistry());
	registry.Iterate<DCore::SpriteComponent>
	(
		[&](DCore::Entity, const DCore::SpriteComponent& spriteComponent) -> bool
		{
			TextureManager::Get().IterateOnTextures
			(
				[&](const uuidType& uuid, const stringType& name) -> bool
				{
					if (uuid == spriteComponent.GetSpriteMaterial().GetDiffuseMapRef().GetUUID())
					{
						m_addedTextures.insert(uuid);
						m_textureNames.push_back({uuid, name});
						return true;
					}
					return false;
				}
			);
			return false;
		}
	);
	if (!s_centerIconSpriteMaterial.IsValid())
	{
		const TextureInfo centerIconTextureInfo(TextureManager::Get().GetTextureInfo(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "SpriteSheetGenCenterIcon.png"));
		uuidType uuid;
		DCore::UUIDGenerator::Get().GenerateUUID(uuid);
		DCore::Texture2DRef centerIconTextureRef(DCore::AssetManager::Get().LoadTexture2D(uuid, centerIconTextureInfo.Binary, centerIconTextureInfo.Sizes, centerIconTextureInfo.NumberOfChannels));
		stbi_image_free(centerIconTextureInfo.Binary);
		DCore::SpriteMaterial spriteMaterial(DCore::SpriteMaterialType::Unlit);
		spriteMaterial.SetDiffuseMapRef(centerIconTextureRef);
		DCore::UUIDGenerator::Get().GenerateUUID(uuid);
		s_centerIconSpriteMaterial = DCore::AssetManager::Get().LoadSpriteMaterial(uuid, std::move(spriteMaterial));
	}
	DCore::ConstructorArgs<DCore::TransformComponent> transformComponentArgs;
	transformComponentArgs.Scale = {0.01f, 0.01f};
	DCore::ConstructorArgs<DCore::SpriteComponent> spriteComponentArgs;
	spriteComponentArgs.DrawOrder = 2;
	spriteComponentArgs.SpriteMaterialRef = s_centerIconSpriteMaterial;
	m_centerIconEntity = registry.CreateEntity<DCore::TransformComponent, DCore::SpriteComponent>(std::make_tuple(transformComponentArgs), std::make_tuple(spriteComponentArgs));
}

SpriteSheetGenPanel::SpriteSheetGenPanel(SpriteSheetGenPanel&& other) noexcept
	:
	m_spriteSheetGen(other.m_spriteSheetGen),
	m_isOpened(other.m_isOpened),
	m_windowFlags(other.m_windowFlags),
	m_renderer(std::move(other.m_renderer)),
	m_cameraComponent(DCore::ConstructorArgs<DCore::PerspectiveCameraComponent>()),
	m_addedTextures(std::move(other.m_addedTextures)),
	m_textureNames(std::move(other.m_textureNames)),
	m_selectedSpriteIndex(other.m_selectedSpriteIndex),
	m_cameraTransformComponent({{0.0f, 0.0f, 50.0f}, 0.0f, {1.0f, 1.0f}}),
	m_drawOnionSkin(other.m_drawOnionSkin),
	m_onionSkinIndex(other.m_selectedSpriteIndex),
	m_centerIconEntity(other.m_centerIconEntity)
{
	other.m_spriteSheetGen.Invalidate();
}

SpriteSheetGenPanel::~SpriteSheetGenPanel()
{
	if (m_isOpened)
	{
		m_renderer.Terminate();
	}
}

void SpriteSheetGenPanel::OpenSpriteSheetGenPanel(spriteSheetGenRefType spriteSheetGen)
{
	s_spriteSheetGenPanels.PushBack(spriteSheetGen);
}

void SpriteSheetGenPanel::RenderPanels()
{
	s_spriteSheetGenPanels.Iterate
	(
		[&](spriteSheetGenPanelContainerType::Ref panel) -> bool
		{
			if (panel->Render())
			{
				s_spriteSheetGenPanels.Remove(panel);
			}
			return false;
		}
	);
}

bool SpriteSheetGenPanel::Render()
{
	if (!m_spriteSheetGen.IsValid())
	{
		m_renderer.Terminate();
		return true;
	}
	if (!ImGui::Begin(m_spriteSheetGen->GetName().c_str(), &m_isOpened, m_windowFlags))
	{
		ImGui::End();
		return false;
	}
	if (!m_isOpened)
	{
		ImGui::End();
		SpriteSheetGenManager::Get().UnloadSpriteSheetGen(m_spriteSheetGen);
		m_renderer.Terminate();
		return true;
	}
	ImVec2 regionAvail(ImGui::GetContentRegionAvail());
	if (ImGui::BeginChild("Config", {regionAvail.x * 0.4f, regionAvail.y}, ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border))
	{
		if (ImGui::Button("Save"))
		{
			m_windowFlags = ImGuiWindowFlags_None;
			SpriteSheetGenManager::Get().SaveSpriteSheetGen(m_spriteSheetGen);
		}
		DrawConfig();
		DrawOnionSkin();
		DrawAddedTextures();
		DrawCenterIconScale();
		DrawAddTexture();
		DrawGenerate();
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("Canvas", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		DrawCanvas();
	}
	ImGui::EndChild();
	ImGui::End();
	return false;
}

void SpriteSheetGenPanel::DrawConfig()
{
	if (ImGui::ArrowButton("##Left", ImGuiDir_Left) && m_selectedSpriteIndex != 0)
	{
		m_selectedSpriteIndex--;
	}
	int selectedSpriteIndex(m_selectedSpriteIndex);
	ImGui::SameLine();
	if (ImGui::SliderInt("##SpriteIndex", &selectedSpriteIndex, 0, m_addedTextures.size() - 1))
	{
		if (selectedSpriteIndex >= 0)
		{
			m_selectedSpriteIndex = selectedSpriteIndex;
		}
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Right", ImGuiDir_Right) && m_addedTextures.size() != 0)
	{
		m_selectedSpriteIndex = std::min(m_selectedSpriteIndex + 1, m_addedTextures.size() - 1);		
	}
	DCore::DVec2 offset(m_addedTextures.size() > 0 ? m_spriteSheetGen->GetSpriteOffset(m_selectedSpriteIndex) : DCore::DVec2(0.0f, 0.0f));
	const ImVec2 regionAvail(ImGui::GetContentRegionAvail());
	const float dragWidth(regionAvail.x * 0.1f);
	if (ImGui::Button("-##X"))
	{
		offset.x--;
		m_windowFlags = ImGuiWindowFlags_UnsavedDocument;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(dragWidth);
	if (ImGui::DragFloat("##OffsetX", &offset.x, 1.0f, -FLT_MAX, FLT_MAX, "%.0f"))
	{
		m_windowFlags = ImGuiWindowFlags_UnsavedDocument;
	}
	ImGui::SameLine();
	if (ImGui::Button("+##X"))
	{
		offset.x++;
		m_windowFlags = ImGuiWindowFlags_UnsavedDocument;
	}
	ImGui::SameLine();
	ImGui::InvisibleButton("##Fill", {regionAvail.x * 0.025f, ImGui::GetItemRectSize().y});
	ImGui::SameLine();
	if (ImGui::Button("-##Y"))
	{
		offset.y--;
		m_windowFlags = ImGuiWindowFlags_UnsavedDocument;
	}
	ImGui::SameLine();
	ImGui::SetNextItemWidth(dragWidth);
	if (ImGui::DragFloat("##OffsetY", &offset.y, 1.0f, -FLT_MAX, FLT_MAX, "%.0f"))
	{
		m_windowFlags = ImGuiWindowFlags_UnsavedDocument;
	}
	ImGui::SameLine();
	if (ImGui::Button("+##Y"))
	{
		offset.y++;
		m_windowFlags = ImGuiWindowFlags_UnsavedDocument;
	}
	if (m_addedTextures.size() > 0)
	{
		m_spriteSheetGen->UpdateSpriteOffset(m_selectedSpriteIndex, offset);
	}
}

void SpriteSheetGenPanel::DrawOnionSkin()
{
	if (!ImGui::CollapsingHeader("Onion Skin"))
	{
		return;
	}
	if (m_addedTextures.size() == 0)
	{
		m_onionSkinIndex = 0;
	}
	else if (m_onionSkinIndex >= m_addedTextures.size())
	{
		m_onionSkinIndex = m_addedTextures.size() - 1;
	}
	ImGui::Indent();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", "Enabled");
	ImGui::SameLine();
	ImGui::Checkbox("##Enabled", &m_drawOnionSkin);
	if (ImGui::ArrowButton("##OnionSkinLeft", ImGuiDir_Left) && m_onionSkinIndex != 0)
	{
		m_onionSkinIndex--;
	}
	int selectedOnionSkinIndex(m_onionSkinIndex);
	ImGui::SameLine();
	if (ImGui::SliderInt("##OnionSkinIndex", &selectedOnionSkinIndex, 0, m_addedTextures.size() - 1))
	{
		if (selectedOnionSkinIndex >= 0)
		{
			m_onionSkinIndex = selectedOnionSkinIndex;
		}
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##OnionSkinRight", ImGuiDir_Right) && m_addedTextures.size() != 0)
	{
		m_onionSkinIndex = std::min(m_onionSkinIndex + 1, m_addedTextures.size() - 1);		
	}
	ImGui::Unindent();
}

void SpriteSheetGenPanel::DrawAddedTextures()
{
	if (!ImGui::CollapsingHeader("Textures"))
	{
		return;
	}
	ImGui::Indent();
	if (ImGui::ArrowButton("##Up", ImGuiDir_Up) && m_selectedSpriteIndex > 0)
	{
		TextureName& onTop(m_textureNames[m_selectedSpriteIndex - 1]);
		TextureName& selected(m_textureNames[m_selectedSpriteIndex]);
		m_spriteSheetGen->UpdateSpriteIndex(m_selectedSpriteIndex, m_selectedSpriteIndex - 1);
		TextureName temp(onTop);
		onTop = selected;
		selected = temp;
		m_selectedSpriteIndex--;
		m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Down", ImGuiDir_Down) && m_selectedSpriteIndex + 1 < m_addedTextures.size())
	{
		TextureName& onBottom(m_textureNames[m_selectedSpriteIndex + 1]);
		TextureName& selected(m_textureNames[m_selectedSpriteIndex]);
		m_spriteSheetGen->UpdateSpriteIndex(m_selectedSpriteIndex, m_selectedSpriteIndex + 1);
		TextureName temp(onBottom);
		onBottom = selected;
		selected = temp;
		m_selectedSpriteIndex++;
		m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
	}
	for (size_t i(0); i < m_textureNames.size(); i++)
	{
		if (ImGui::Selectable(m_textureNames[i].Name.Data(), i == m_selectedSpriteIndex))
		{
			m_selectedSpriteIndex = i;
		}
		if (ImGui::BeginPopupContextItem())
		{
			m_selectedSpriteIndex = i;
			if (ImGui::Selectable("Delete"))
			{
				m_addedTextures.erase(m_textureNames[i].UUID);
				m_textureNames.erase(m_textureNames.begin() + m_selectedSpriteIndex);
				m_spriteSheetGen->RemoveSprite(m_selectedSpriteIndex);
				if (m_selectedSpriteIndex > 0)
				{
					m_selectedSpriteIndex--;
				}
				m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
				ImGui::EndPopup();
				break;
			}
			ImGui::EndPopup();
		}
	}
	ImGui::Unindent();
}

void SpriteSheetGenPanel::DrawCenterIconScale()
{
	if (!ImGui::CollapsingHeader("Aim Scale"))
	{
		return;
	}
	registryType& registry(m_spriteSheetGen->GetRegistry());
	DCore::TransformComponent& transfromComponent(registry.GetComponents<DCore::TransformComponent>(m_centerIconEntity));
	float scale(transfromComponent.GetScale().x);
	ImGui::Indent();
	if (ImGui::DragFloat("##CenterIconScale", &scale, 0.01f, 0.01f, FLT_MAX))
	{
		transfromComponent.SetScale({scale, scale});
	}
	ImGui::Unindent();
}

void SpriteSheetGenPanel::DrawAddTexture()
{
	if (!ImGui::CollapsingHeader("Not Added Textures"))
	{
		return;
	}
	ImGui::Indent();
	TextureManager::Get().IterateOnTextures
	(
		[&](const uuidType& uuid, const stringType& name) -> bool
		{
			if (m_addedTextures.find(uuid) != m_addedTextures.end())
			{
				return false;
			}
			if (ImGui::Selectable(name.c_str()))
			{
				m_spriteSheetGen->AddSprite(uuid);
				m_addedTextures.insert(uuid);
				m_textureNames.push_back({uuid, name});
				m_windowFlags = ImGuiWindowFlags_UnsavedDocument;
				return true;
			}
			return false;
		}
	);
	ImGui::Unindent();
}

void SpriteSheetGenPanel::DrawCanvas()
{
	const ImVec2 regionAvail(ImGui::GetContentRegionAvail());
	if (regionAvail.x == 0.0f || regionAvail.y == 0.0f)
	{
		return;
	}
	if (m_renderer.IsRenderingDone())
	{
		m_renderer.Begin({regionAvail.x, regionAvail.y});
		m_renderer.SetClearColor({1.0f, 1.0f, 1.0f, 1.0f});
		DCore::Registry& registry(m_spriteSheetGen->GetRegistry());
		if (m_addedTextures.size() != 0)
		{
			{
				SpriteSheetElement spriteSheetElement(m_spriteSheetGen->GetSpriteSheetElement(m_selectedSpriteIndex));
				SubmitEntity(spriteSheetElement.Entity, registry, {regionAvail.x, regionAvail.y});
			}
			if (m_drawOnionSkin)
			{
				SpriteSheetElement spriteSheetElement(m_spriteSheetGen->GetSpriteSheetElement(m_onionSkinIndex));
				entityType entity(spriteSheetElement.Entity);
				const DCore::SpriteComponent& spriteComponent(registry.GetComponents<DCore::SpriteComponent>(entity));
				dVec4 tintColor(spriteComponent.GetTintColor());
				tintColor.a = 0.5f;
				dUInt drawOrder(0);
				SubmitEntity(entity, registry, {regionAvail.x, regionAvail.y}, &tintColor, &drawOrder);
			}
		}
		SubmitEntity(m_centerIconEntity, registry, {regionAvail.x, regionAvail.y});
		m_renderer.Render();
	}
	ImGui::Image((ImTextureID)(uintptr_t)m_renderer.GetOutputTextureId(), {regionAvail.x, regionAvail.y}, {0, 1}, {1, 0});
	// TODO. Draw center indicator.
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	// Camera movement
	if (ImGui::IsWindowHovered())
	{
		ImGuiIO& io(ImGui::GetIO());
		m_cameraTransformComponent.AddTranslation({0.0f, 0.0f, -io.MouseWheel * 50.0f});
		static ImVec2 lastMousePos;
		const ImVec2& mousePos(io.MousePos);
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			const DCore::DVec2 deltaMousePos({mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y});
			const float halfFovY(glm::radians(m_cameraComponent.GetFOV() / 2.0f));
			const float tanHalfFovY(glm::tan(halfFovY));
			const float dY(2 * m_cameraComponent.GetNear() * tanHalfFovY);
			const float DY(2 * m_cameraTransformComponent.GetTranslation().z * tanHalfFovY);
			const float unitsPerPixelY(dY / regionAvail.y);
			const float deltaY(unitsPerPixelY * deltaMousePos.y * DY / dY);
			const float halfFovX(glm::atan(regionAvail.x / regionAvail.y * dY / (2 * m_cameraComponent.GetNear())));
			const float tanHalfFovX(glm::tan(halfFovX));
			const float dX(2 * m_cameraComponent.GetNear() * tanHalfFovX);
			const float DX(2 * m_cameraTransformComponent.GetTranslation().z * tanHalfFovX);
			const float unitsPerPixelX(dX / regionAvail.x);
			const float deltaX(unitsPerPixelX * deltaMousePos.x * DX / dX);
			m_cameraTransformComponent.AddTranslation({-deltaX, deltaY, 0.0f});
		}
		lastMousePos = mousePos;
	}
}

void SpriteSheetGenPanel::DrawGenerate()
{
	if (!ImGui::CollapsingHeader("Generation"))
	{
		return;
	}
	int numberOfColuns(m_spriteSheetGen->GetNumberOfColumns());
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Number of columns");
	ImGui::SameLine();
	if (ImGui::DragInt("##Coluns", &numberOfColuns, 1.0f, 1, INT_MAX))
	{
		m_spriteSheetGen->SetNumberOfColumns(numberOfColuns);
		m_windowFlags |= ImGuiWindowFlags_UnsavedDocument;
	}
	if (ImGui::Button("Generate") && m_addedTextures.size() > 0)
	{
		bool canGenerateSpriteSheet(true);
		registryType& registry(m_spriteSheetGen->GetRegistry());
		m_spriteSheetGen->IterateOnElements
		(
			[&](SpriteSheetElement& element) -> bool
			{
				entityType entity(element.Entity);
				const DCore::SpriteComponent& spriteComponent(registry.GetComponents<DCore::SpriteComponent>(entity));
				if (!spriteComponent.GetSpriteMaterial().GetDiffuseMapRef().IsValid())
				{
					canGenerateSpriteSheet = false;
					return true;
				}
				return false;
			}
		);
		if (!canGenerateSpriteSheet)
		{
			Log::Get().TerminalLog("%s", "Cannot generate a sprite sheet: there is a invalid texture reference.");
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot generate a sprite sheet: there is a invalid texture reference.");
			return;
		}
		const ImVec2 regionAvail(ImGui::GetContentRegionAvail());
		if (regionAvail.x == 0.0f || regionAvail.y == 0.0f)
		{
			return;
		}
		// Calculate cell size.
		float maxRight(0.0f);
		float maxLeft(0.0f);
		float maxTop(0.0f);
		float maxBottom(0.0f);
		bool isFirstIteration(true);
		m_spriteSheetGen->IterateOnElements
		(
			[&](SpriteSheetElement& element) -> bool
			{
				entityType entity(element.Entity);
				auto [transformComponent, spriteComponent] = registry.GetComponents<DCore::TransformComponent, DCore::SpriteComponent>(entity);
				const DCore::DVec3 translation(transformComponent->GetTranslation());
				DCore::ReadWriteLockGuard materialGuard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
				DCore::ReadWriteLockGuard textureGuard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
				const dVec2 textureSizes(spriteComponent->GetSpriteMaterial().GetDiffuseMapRef().GetDimensions());
				const float halfTextureWidth(textureSizes.x/2.0f);
				const float halfTextureHeight(textureSizes.y/2.0f);
				const float right(translation.x + halfTextureWidth);
				const float left(translation.x - halfTextureWidth);
				const float top(translation.y + halfTextureHeight);
				const float bottom(translation.y - halfTextureHeight);
				if (isFirstIteration)
				{
					maxRight = right;
					maxLeft = left;
					maxTop = top;
					maxBottom = bottom;
					isFirstIteration = false;
					return false;
				}
				maxRight = right > maxRight ? right : maxRight;
				maxLeft = left < maxLeft ? left : maxLeft;
				maxTop = top > maxTop ? top : maxTop;
				maxBottom = bottom < maxBottom ? bottom : maxBottom;
				return false;
			}
		);
		const dVec2 cellSizes({maxRight - maxLeft, maxTop - maxBottom});
		// Calculate texture size.
		const float numberOfLines(glm::ceil(static_cast<float>(m_spriteSheetGen->GetNumberOfElements()) / static_cast<float>(numberOfColuns)));
		const dVec2 textureSizes{numberOfColuns * cellSizes.x, numberOfLines * cellSizes.y};
		const dVec2 halfTextureSizes(textureSizes / 2.0f);
		dMat4 viewProjectionMatrix(1.0f);
		viewProjectionMatrix = glm::translate(viewProjectionMatrix, {halfTextureSizes.x + maxLeft, -halfTextureSizes.y + maxTop, 0.5f});
		viewProjectionMatrix = glm::inverse(viewProjectionMatrix);
		viewProjectionMatrix = glm::ortho(-halfTextureSizes.x, halfTextureSizes.x, -halfTextureSizes.y, halfTextureSizes.y) * viewProjectionMatrix;
		while(!m_renderer.IsRenderingDone());
		// Submit entities (calculate entities positions).
		size_t currentColumnIndex(0);
		size_t currentLineIndex(0);
		m_renderer.Begin({textureSizes.x, textureSizes.y});
		m_renderer.SetClearColor({0.0f, 0.0f, 0.0f, 0.0f});
		m_spriteSheetGen->IterateOnElements
		(
			[&](SpriteSheetElement& element) -> bool
			{
				entityType entity(element.Entity);
				DCore::TransformComponent& transformComponent(registry.GetComponents<DCore::TransformComponent>(entity));
				const DCore::DVec3 offset(transformComponent.GetTranslation());
				const DCore::DVec3 inSpriteSheetOffset{offset.x + currentColumnIndex * cellSizes.x, offset.y - currentLineIndex * cellSizes.y, 0.0f};
				transformComponent.SetTranslation(inSpriteSheetOffset);
				SubmitEntity(entity, registry, {textureSizes.x, textureSizes.y}, nullptr, nullptr, &viewProjectionMatrix);
				transformComponent.SetTranslation(offset);
				currentColumnIndex++;
				if (currentColumnIndex >= numberOfColuns)
				{
					currentColumnIndex = 0;
					currentLineIndex++;
				}
				return false;
			}
		);
		m_renderer.Render();
		// Wait draw to complete.
		while(!m_renderer.IsRenderingDone());
		// Select file location.
		std::filesystem::path filePath;
		if (FileBrowser::Get().SaveFile(filePath))
		{
			const size_t textureSize(textureSizes.x * textureSizes.y * 4 * sizeof(GLuint));
			unsigned char* imageBuffer(new unsigned char[textureSize]);
			glBindTexture(GL_TEXTURE_2D, m_renderer.GetOutputTextureId()); CHECK_GL_ERROR;
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer); CHECK_GL_ERROR;
			stbi_flip_vertically_on_write(true);
			stbi_write_png(filePath.string().c_str(), textureSizes.x, textureSizes.y, 4, imageBuffer, textureSizes.x * 4);
			delete[] imageBuffer;
		}
	}
}

void SpriteSheetGenPanel::SubmitEntity(entityType entity, registryType& registry, const dVec2& viewportSizes, const dVec4* overrideTintColor, const dUInt* overrideDrawOrder, const dMat4* overrideViewPorjectionMatrix)
{
	DCore::ReadWriteLockGuard materialGuard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
	DCore::ReadWriteLockGuard textureGuard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
	const auto [transformComponent, spriteComponent] = registry.GetComponents<DCore::TransformComponent, DCore::SpriteComponent>(entity);
	DCore::DMat4 mvp((overrideViewPorjectionMatrix == nullptr ? (m_cameraComponent.GetProjectionMatrix(viewportSizes) * glm::inverse(m_cameraTransformComponent.GetModelMatrix())) : *overrideViewPorjectionMatrix) * transformComponent->GetModelMatrix());
	DCore::Quad3 vertexPostions;
	const DCore::Quad2 spriteVertexPositions(spriteComponent->GetCurrentSpriteVertexPositions());
	vertexPostions.BottomLeft = {spriteVertexPositions.BottomLeft.x, spriteVertexPositions.BottomLeft.y, 0.0f};
	vertexPostions.BottomRight = {spriteVertexPositions.BottomRight.x, spriteVertexPositions.BottomRight.y, 0.0f};
	vertexPostions.TopRight = {spriteVertexPositions.TopRight.x, spriteVertexPositions.TopRight.y, 0.0f};
	vertexPostions.TopLeft = {spriteVertexPositions.TopLeft.x, spriteVertexPositions.TopLeft.y, 0.0f};
	const DCore::DVec4 diffuseColor(spriteComponent->GetDiffuseColor());
	const DCore::SceneIdType sceneId(0);
	const DCore::SceneVersionType sceneVersion(0);
	bool toUseDiffuseTexture(true);
	uint32_t diffuseTextureId(0);
	if (!spriteComponent->GetSpriteMaterial().IsValid())
	{
		toUseDiffuseTexture = false;
	}
	else
	{
		if (spriteComponent->GetSpriteMaterial().GetDiffuseMapRef().IsValid())
		{
			diffuseTextureId = spriteComponent->GetSpriteMaterial().GetDiffuseMapRef().GetId();
		}
		else
		{
			toUseDiffuseTexture = false;
		}
	}
	const DCore::Quad2 spriteUvs(spriteComponent->GetCurrentSpriteUvs());
	const DCore::DVec4& tintColor(overrideTintColor ==  nullptr ? spriteComponent->GetTintColor() : *overrideTintColor);
	DCore::Renderer::unlitTexturedObjectRendererType::quadType quad;
	for (uint8_t index(0); index < 4; index++)
	{
		DCore::UnlitTexturedVertex& vertex(quad[index]);
		vertex.DrawOrder = overrideDrawOrder == nullptr ? spriteComponent->GetDrawOrder() : *overrideDrawOrder;
		vertex.MVP = mvp;
		vertex.VertexPos = vertexPostions.At(index);
		vertex.DiffuseColor = diffuseColor;
		vertex.TintColor = tintColor;
		vertex.ToUseDiffuseTex = toUseDiffuseTexture ? 1 : 0;
		vertex.DiffuseTexId = diffuseTextureId;
		vertex.UV = spriteUvs.At(index);
		vertex.EntityId = entity.GetId();
		vertex.EntityVersion = entity.GetVersion();
		vertex.SceneId = sceneId;
		vertex.SceneVersion = sceneVersion;
	}
	m_renderer.SubmitUnlitTexturedObject(quad);
}

}
