#pragma once

#include "DommusCore.h"

#include <filesystem>
#include <functional>
#include <unordered_set>


namespace DEditor
{

struct TextureInfo
{
	using dVec2 = DCore::DVec2;

	unsigned char* Binary;
	dVec2 Sizes;
	int NumberOfChannels;
};

class TextureManager
{
public:
	using uuidType = DCore::UUIDType;
	using textureRefType = DCore::Texture2DRef;
	using lockDataType = DCore::LockData;
	using stringType = std::string;
	using pathType = std::filesystem::path;
	using textureIterationCallbackType = std::function<bool(const uuidType&, const stringType&)>;
	using loadingTexturesSetType = std::unordered_set<uuidType>;
public:
	~TextureManager() = default;
public:
	static TextureManager& Get()
	{
		static TextureManager textureManager;
		return textureManager;
	}
public:
	void ImportTexture(const pathType& outsidePath, const pathType& thumbnailDirectory);
	[[nodiscard]] DCore::Texture2DRef LoadTexture2D(const DCore::UUIDType&);
	[[nodiscard]] DCore::Texture2D LoadRawTexture2D(const pathType&);
	DCore::Texture2D LoadRawTexture2D(const pathType&, unsigned char** outBinary); 
	TextureInfo GetTextureInfo(const pathType&);
	void DeleteTexture(const uuidType&);
	bool RenameTexture(const uuidType&, const stringType& newName);
	void SaveTexture(textureRefType);
	void IterateOnTextures(textureIterationCallbackType);
	bool TextureExists(const uuidType&);
private:
	TextureManager(); 
private:
	lockDataType m_lockData;
	loadingTexturesSetType m_texturesLoading;
private:
	pathType GetTextureDirectoryPath() const;
	void SaveTexturesMap() const;
	void CreateTextureThumbnail(const pathType& thumbnailPath, const DCore::DString&) const;
	DCore::Texture2DMetadata GetTexture2DMetadata(const DCore::DString& uuidString);
};

}
