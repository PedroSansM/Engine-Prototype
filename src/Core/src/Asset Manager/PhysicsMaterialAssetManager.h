#pragma once

#include "AssetManagerTypes.h"
#include "PhysicsMaterial.h"
#include "UUID.h"
#include "ReadWriteLockGuard.h"

#include <unordered_map>
#include <string>



namespace DCore
{

class PhysicsMaterialAssetManager
{
	friend class ReadWriteLockGuard;
public:
	using physicsMaterialContainerType = AssetContainerType<PhysicsMaterial>;	
	using loadedPhysicsMaterialsRefType = std::unordered_map<UUIDType, InternalPhysicsMaterialRefType>;
public:
	PhysicsMaterialAssetManager(const PhysicsMaterialAssetManager&) = delete;
	PhysicsMaterialAssetManager(PhysicsMaterialAssetManager&&) = delete;
public:
	virtual ~PhysicsMaterialAssetManager();
public:
	bool IsPhysicsMaterialLoaded(const UUIDType&);
	[[nodiscard]] PhysicsMaterialRef LoadPhysicsMaterial(const UUIDType&, PhysicsMaterial&&);
	[[nodiscard]] PhysicsMaterialRef GetPhysicsMaterial(const UUIDType&);
	void UnloadPhysicsMaterial(const UUIDType&, bool removeAllReferences = false);
protected:
	PhysicsMaterialAssetManager() = default;
private:
	physicsMaterialContainerType m_physicsMaterials;
	loadedPhysicsMaterialsRefType m_loadedPhysicsMaterials;
	LockData m_lockData;
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
