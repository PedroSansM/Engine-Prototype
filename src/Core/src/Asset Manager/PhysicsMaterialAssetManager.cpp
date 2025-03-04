#include "PhysicsMaterialAssetManager.h"
#include "DCoreAssert.h"
#include "Asset.h"



namespace DCore
{

PhysicsMaterialAssetManager::~PhysicsMaterialAssetManager()
{
	m_physicsMaterials.Clear();
	m_loadedPhysicsMaterials.clear();
}

bool PhysicsMaterialAssetManager::IsPhysicsMaterialLoaded(const UUIDType& uuid)
{
	return m_loadedPhysicsMaterials.find(uuid) != m_loadedPhysicsMaterials.end();
}

PhysicsMaterialRef PhysicsMaterialAssetManager::LoadPhysicsMaterial(const UUIDType& uuid, PhysicsMaterial&& physicsMaterial)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	DASSERT_E(m_loadedPhysicsMaterials.find(uuid) == m_loadedPhysicsMaterials.end());
	InternalPhysicsMaterialRefType internalRef(m_physicsMaterials.PushBack(uuid, std::move(physicsMaterial)));
	m_loadedPhysicsMaterials.insert({uuid, internalRef});
	return PhysicsMaterialRef(internalRef, m_lockData);
}

PhysicsMaterialRef PhysicsMaterialAssetManager::GetPhysicsMaterial(const UUIDType& uuid)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	DASSERT_E(m_loadedPhysicsMaterials.find(uuid) != m_loadedPhysicsMaterials.end());
	InternalPhysicsMaterialRefType internalRef(m_loadedPhysicsMaterials.find(uuid)->second);
	internalRef->AddReferenceCount();
	return PhysicsMaterialRef(internalRef, m_lockData);
}

void PhysicsMaterialAssetManager::UnloadPhysicsMaterial(const UUIDType& uuid, bool removeAllReferences)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	auto it(m_loadedPhysicsMaterials.find(uuid));
	if (it == m_loadedPhysicsMaterials.end())
	{
		return;
	}
	InternalPhysicsMaterialRefType internalRef(it->second);
	if (internalRef->GetReferenceCount() == 1 || removeAllReferences)
	{
		m_loadedPhysicsMaterials.erase(uuid);
		m_physicsMaterials.Remove(internalRef);
		return;
	}
	internalRef->SubReferenceCount();
}

}
