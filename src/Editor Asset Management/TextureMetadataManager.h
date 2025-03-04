#pragma once

#include "DommusCore.h"

#include <filesystem>



namespace DEditor
{

class TextureMetadataManager
{
public:
	~TextureMetadataManager() = default;
public:
	inline static TextureMetadataManager& Get()
	{
		static TextureMetadataManager textureMetadataManager;
		return textureMetadataManager;
	}
public:
	void AddTextureMetadataFile(const std::filesystem::path& thumbnailDirectory, const DCore::UUIDType& uuid, const std::filesystem::path& texturePath, const DCore::DString& textureName);
	DCore::Texture2DMetadata GetTextureMetadata(const DCore::UUIDType&);
private:
	TextureMetadataManager();
private:
	std::filesystem::path GetMetadatasPath() const;
	void GenerateTextureThumbnail(const std::filesystem::path& thumbnailDirectory, const std::filesystem::path& textureRelativePath, const DCore::UUIDType& uuid, const DCore::DString& textureName) const;
};

}
