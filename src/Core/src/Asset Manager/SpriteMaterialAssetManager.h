#pragma once

#include "UUID.h"
#include "SpriteMaterial.h"
#include "ReadWriteLockGuard.h"
#include "AssetManagerTypes.h"

#include <thread>
#include <unordered_set>
#include <mutex>



namespace DCore
{

class SpriteMaterialAssetManager
{
	friend class ReadWriteLockGuard;
public:
	using spriteMaterialContainerType = AssetContainerType<SpriteMaterial>;
	using loadedSpriteMaterialContainerType = std::unordered_map<UUIDType, InternalSpriteMaterialRefType>;
public:
	virtual ~SpriteMaterialAssetManager();
public:	
	bool IsSpriteMaterialLoaded(const UUIDType&);
	[[nodiscard]] SpriteMaterialRef LoadSpriteMaterial(const UUIDType&, SpriteMaterial&&);
	[[nodiscard]] SpriteMaterialRef GetSpriteMaterial(const UUIDType&);
	void UnloadSpriteMaterial(const UUIDType&, bool removeAllReferences = false);
protected:
	SpriteMaterialAssetManager() = default;
private:
	spriteMaterialContainerType m_spriteMaterials;
	loadedSpriteMaterialContainerType m_loadedSpriteMaterials;
	LockData m_lockData;
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
