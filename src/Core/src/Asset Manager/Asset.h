#pragma once

#include "UUID.h"

#include <algorithm>
#include <cstdint>
#include <mutex>



namespace DCore
{

template <class AssetType>
class Asset
{
	friend class AssetManager;
	friend class SceneAssetManager;
	friend class Texture2DAssetManager;
	friend class SpriteMaterialAssetManager;
	friend class AnimationAssetManager;
	
	template <class DataType, class IdType, class VersionType>
	friend class ReciclingVector;
public:
	using assetType = AssetType;
public:
	Asset(const UUIDType& uuid, AssetType&& asset) noexcept
		:
		m_asset(std::move(asset)),
		m_uuid(uuid),
		m_referenceCount(1)
	{}
	
	Asset(const Asset& other)
		:
		m_asset(other.m_asset),
		m_uuid(other.m_uuid),
		m_referenceCount(other.m_referenceCount)
	{}

	Asset(Asset&& other) noexcept
		:
		m_asset(std::move(other.m_asset)),
		m_uuid(other.m_uuid),
		m_referenceCount(other.m_referenceCount)
	{}

	~Asset() = default;
public:
	AssetType& GetAsset()
	{
		return m_asset;
	}

	const AssetType& GetAsset() const
	{
		return m_asset;
	}

	const UUIDType& GetUUID() const
	{
		return m_uuid;
	}

	void AddReferenceCount()
	{
		m_referenceCount++;
	}

	void SubReferenceCount()
	{
		m_referenceCount--;
	}

	size_t GetReferenceCount() const
	{
		return m_referenceCount;
	}
public:
	Asset& operator=(Asset&& other) noexcept
	{
		m_asset = std::move(other.m_asset);
		m_uuid = other.m_uuid;
		m_referenceCount = other.m_referenceCount;
		return *this;
	}
private:
	AssetType m_asset;
	UUIDType m_uuid;
	size_t m_referenceCount;
};

template <class RefType, class RefIdType>
class AssetRef
{
public:
	AssetRef(RefType ref, std::recursive_mutex& mutex)
		:
		m_ref(ref),
		m_mutex(mutex)
	{}
	AssetRef(const AssetRef& other)
		:
		m_ref(other.m_ref),
		m_mutex(other.m_mutex)
	{}
	AssetRef(AssetRef&& other)
		:
		m_ref(other.m_ref),
		m_mutex(other.m_mutex)
	{}
	~AssetRef() = default;
public:
	bool IsValid()
	{
		std::lock_guard<std::recursive_mutex> guard(m_mutex);
		return m_ref.IsValid();
	}
	
	RefIdType GetId()
	{
		return m_ref.GetId();
	}

	void Lock()
	{
		m_mutex.lock();
	}

	RefType& GetInternalRef()
	{
		return m_ref;
	}

	void Unlock()
	{
		m_mutex.unlock();
	}
public:
	RefType& operator->()
	{
		return m_ref;
	}

	AssetRef& operator=(const AssetRef& other)
	{
		m_ref = other.m_ref;
		return *this;
	}

	bool operator==(const AssetRef& other)
	{
		return m_ref == other.m_ref;
	}
private:
	RefType m_ref;
	std::recursive_mutex& m_mutex;
};

template <class AssetRef>
class AssetRefGuard
{
public:
	AssetRefGuard(AssetRef assetRef)
		:
		m_assetRef(assetRef)
	{
		m_assetRef.Lock();
	}

	~AssetRefGuard()
	{
		m_assetRef.Unlock();
	}
private:
	AssetRef m_assetRef;
};

}
