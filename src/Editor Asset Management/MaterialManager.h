#pragma once

#include "DommusCore.h"

#include <filesystem>
#include <string>
#include <unordered_map>



namespace DEditor
{

class MaterialManager
{
public:
	using uuidType = DCore::UUIDType;
	using spriteMaterialType = DCore::SpriteMaterialType;
	using spriteMaterialRefType = DCore::SpriteMaterialRef;
	using stringType = std::string;
	using pathType = std::filesystem::path;
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
	pathType GetMaterialsPath() const;
	void GenerateSpriteMaterialThumbail(const pathType& thumbailPath, const stringType& uuidString, const stringType& spriteMaterialTypeString, const stringType& materialName);
	void SaveMaterialsMap();
};

}
