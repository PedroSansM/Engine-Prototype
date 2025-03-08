#pragma once

#include "DommusCore.h"

#include <filesystem>
#include <string>
#include <unordered_set>
#include <mutex>



namespace DEditor
{

class MaterialManager
{
public:
	using uuidType = DCore::UUIDType;
	using spriteMaterialType = DCore::SpriteMaterialType;
	using spriteMaterialRefType = DCore::SpriteMaterialRef;
	using lockDataType = DCore::LockData;
	using stringType = std::string;
	using pathType = std::filesystem::path;
	using materialsLoadingSetType = std::unordered_set<uuidType>;
public:
	~MaterialManager() = default;
public:
	static MaterialManager& Get()
	{
		static MaterialManager materialManager;
		return materialManager;
	}
public:
	void CreateSpriteMaterial(spriteMaterialType, const stringType& materialName, const pathType& thumbnailDirectory);
	[[nodiscard]] spriteMaterialRefType LoadSpriteMaterial(const uuidType&);
	void RemoveTextureReference(const uuidType& textureUUID);
	void SaveSpriteMaterial(const spriteMaterialRefType);
	void DeleteSpriteMaterial(const uuidType& materialUUID);
	bool RenameSpriteMaterial(const uuidType&, const stringType& newName);
	bool SpriteMaterialExists(const uuidType&);
private:
	MaterialManager();
private:
	lockDataType m_lockData;
	materialsLoadingSetType m_resourcesLoading;
private:
	pathType GetMaterialsPath() const;
	void GenerateSpriteMaterialThumbnail(const pathType& thumbailPath, const stringType& uuidString, const stringType& spriteMaterialTypeString, const stringType& materialName);
	void SaveMaterialsMap();
};

}
