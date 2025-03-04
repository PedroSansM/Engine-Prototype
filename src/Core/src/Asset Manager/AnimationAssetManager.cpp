#include "AnimationAssetManager.h"
#include "DCoreAssert.h"
#include "ReadWriteLockGuard.h"



namespace DCore
{

AnimationAssetManager::~AnimationAssetManager()
{
	m_animations.Clear();
	m_loadedAnimations.clear();
}

bool AnimationAssetManager::IsAnimationLoaded(const UUIDType& uuid)
{
	return m_loadedAnimations.find(uuid) != m_loadedAnimations.end();
}

AnimationRef AnimationAssetManager::LoadAnimation(const UUIDType& uuid, Animation&& animation)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);	
	DASSERT_E(m_loadedAnimations.find(uuid) == m_loadedAnimations.end());
	InternalAnimationRefType internalRef(m_animations.PushBack(uuid, std::move(animation)));
	m_loadedAnimations.insert({uuid, internalRef});
	return AnimationRef(internalRef, m_lockData);
}

AnimationRef AnimationAssetManager::GetAnimation(const UUIDType& uuid)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);	
	DASSERT_E(m_loadedAnimations.find(uuid) != m_loadedAnimations.end());
	InternalAnimationRefType internalRef(m_loadedAnimations.find(uuid)->second);
	internalRef->AddReferenceCount();
	return AnimationRef(internalRef, m_lockData);
}

void AnimationAssetManager::UnloadAnimation(const UUIDType& uuid, bool removeAllReferences)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	auto it(m_loadedAnimations.find(uuid));
	if (it == m_loadedAnimations.end())
	{
		return;
	}
	InternalAnimationRefType internalRef(it->second);
	if (internalRef->m_referenceCount == 1 || removeAllReferences)
	{
		m_loadedAnimations.erase(uuid);
		m_animations.Remove(internalRef);
		return;
	}
	internalRef->SubReferenceCount();
}

void AnimationAssetManager::RenameAnimation(const UUIDType& uuid, const stringType& newName)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	auto it(m_loadedAnimations.find(uuid));
	DASSERT_E(it != m_loadedAnimations.end());
	it->second->GetAsset().SetName(newName);
}

}
