#include "SpriteMaterialAssetManager.h"
#include "AssetManagerTypes.h"
#include "SpriteMaterial.h"
#include "AssetManager.h"



namespace DCore
{

SpriteMaterialAssetManager::~SpriteMaterialAssetManager()
{
	m_spriteMaterials.Clear();
	m_loadedSpriteMaterials.clear();
}

bool SpriteMaterialAssetManager::IsSpriteMaterialLoaded(const UUIDType& uuid)
{
	return m_loadedSpriteMaterials.find(uuid) != m_loadedSpriteMaterials.end();
}

SpriteMaterialRef SpriteMaterialAssetManager::LoadSpriteMaterial(const UUIDType& uuid, SpriteMaterial&& spriteMaterial)
{
	if (m_loadedSpriteMaterials.find(uuid) != m_loadedSpriteMaterials.end())
	{
		return GetSpriteMaterial(uuid);
	}
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	InternalSpriteMaterialRefType internalRef(m_spriteMaterials.PushBack(uuid, std::move(spriteMaterial)));
	m_loadedSpriteMaterials.insert({uuid, internalRef});
	return SpriteMaterialRef(internalRef, m_lockData);
}

SpriteMaterialRef SpriteMaterialAssetManager::GetSpriteMaterial(const UUIDType& uuid)
{
	DASSERT_E(m_loadedSpriteMaterials.find(uuid) != m_loadedSpriteMaterials.end());
	InternalSpriteMaterialRefType internalRef(m_loadedSpriteMaterials.find(uuid)->second);
	ReadWriteLockGuard spriteMaterialGuard(LockType::WriteLock, m_lockData);
	ReadWriteLockGuard textureGuard(LockType::WriteLock, *static_cast<Texture2DAssetManager*>(&AssetManager::Get()));
	internalRef->AddReferenceCount();
	if (internalRef->GetAsset().GetDiffuseMapRef().IsValid())
	{
		internalRef->GetAsset().GetDiffuseMapRef().GetInternalRef()->AddReferenceCount();
	}
	if (internalRef->GetAsset().GetAmbientMapRef().IsValid())
	{
		internalRef->GetAsset().GetAmbientMapRef().GetInternalRef()->AddReferenceCount();
	}
	if (internalRef->GetAsset().GetSpecularMapRef().IsValid())
	{
		internalRef->GetAsset().GetSpecularMapRef().GetInternalRef()->AddReferenceCount();
	}
	return SpriteMaterialRef(internalRef, m_lockData);
}

void SpriteMaterialAssetManager::UnloadSpriteMaterial(const UUIDType& uuid, bool removeAllReferences)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	auto it(m_loadedSpriteMaterials.find(uuid));
	if (it == m_loadedSpriteMaterials.end())
	{
		return;
	}
	InternalSpriteMaterialRefType internalRef(it->second);
	if (internalRef->m_referenceCount == 1 || removeAllReferences)
	{
		m_loadedSpriteMaterials.erase(uuid);
		constexpr uint8_t numberOfTextures(3);
		Texture2DRef textures[numberOfTextures]{
			internalRef->GetAsset().GetDiffuseMapRef(), 
			internalRef->GetAsset().GetAmbientMapRef(), 
			internalRef->GetAsset().GetSpecularMapRef()};  
		for (uint8_t i(0); i < numberOfTextures; i++)
		{
			textures[i].Unload();
		}
		m_spriteMaterials.Remove(internalRef);
		return;
	}
	internalRef->SubReferenceCount();
}

}
