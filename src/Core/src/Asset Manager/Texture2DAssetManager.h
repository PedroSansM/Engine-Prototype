#pragma once

#include "AssetManagerTypes.h"
#include "ReadWriteLockGuard.h"
#include "UUID.h"
#include "Texture2D.h"

#include <unordered_set>
#include <mutex>
#include <thread>



namespace DCore
{

class Texture2DAssetManager
{
	friend class ReadWriteLockGuard;
public:
	using texture2DContainerType = AssetContainerType<Texture2D>;
	using loadedTexture2DContainerType = std::unordered_map<UUIDType, InternalTexture2DRefType>;
public:
	virtual ~Texture2DAssetManager();
public:
	bool IsTexture2DLoaded(const UUIDType&);
	[[nodiscard]] Texture2DRef LoadTexture2D(const UUIDType& uuid, unsigned char* binary, const DVec2& size, int numberChannels, Texture2DMetadata metadata = Texture2DMetadata());
	[[nodiscard]] Texture2D LoadRawTexture2D(unsigned char* binary, const DVec2& size, int numberChannels, Texture2DMetadata metadata = Texture2DMetadata());
	[[nodiscard]] Texture2DRef GetTexture2DRef(const UUIDType&);
	void UnloadTexture2D(const UUIDType&, bool removeAllReferences = false);
protected:
	Texture2DAssetManager() = default;
private:
	texture2DContainerType m_textures;
	loadedTexture2DContainerType m_loadedTextures2D;
	LockData m_lockData;
private:
	Texture2D GenerateTexture2D(unsigned char* binary, const DVec2& size, int numberChannels, Texture2DMetadata metadata = Texture2DMetadata());
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
