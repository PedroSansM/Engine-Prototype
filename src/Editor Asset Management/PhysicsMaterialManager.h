#pragma once

#include "DommusCore.h"

#include <string>
#include <filesystem>



namespace DEditor
{

class PhysicsMaterialManager
{
public:
	using physicsMaterialRefType = DCore::PhysicsMaterialRef;
	using uuidType = DCore::UUIDType;
	using stringType = std::string;
	using pathType = std::filesystem::path;
public:
	~PhysicsMaterialManager() = default;
public:;
	static PhysicsMaterialManager& Get()
	{
		static PhysicsMaterialManager manager;
		return manager;
	}
public:
	void CreatePhysicsMaterial(const stringType& physicsMaterialName, const pathType& thumbnailDirectoryPath);
	[[nodiscard]] physicsMaterialRefType LoadPhysicsMaterial(const uuidType&);
	void SaveChanges(const physicsMaterialRefType);
	void DeletePhysicsMaterial(const uuidType& phyiscsMaterialUUID);
	bool RenamePhysicsMaterial(const uuidType& uuid, const stringType& newName);
private:
	PhysicsMaterialManager();
private:
	pathType GetPhysicsMaterialsPath() const;
	void GeneratePhysicsMaterialThumbnail(const pathType& thumbailPath, const stringType& uuidString, const stringType& physicsMaterialName);
	void SavePhysicsMaterialMap();
};

}
