#pragma once

#include "AssetManagerTypes.h"
#include "UUID.h"
#include "Animation.h"

#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <string>



namespace DCore
{

class AnimationAssetManager
{
	friend class ReadWriteLockGuard;
public:
	using animationContainerType = AssetContainerType<Animation>;
	using loadedAnimationsRefType = std::unordered_map<UUIDType, InternalAnimationRefType>;
	using stringType = std::string;
public:
	virtual ~AnimationAssetManager();
public:
	bool IsAnimationLoaded(const UUIDType&);
	[[nodiscard]] AnimationRef LoadAnimation(const UUIDType&, Animation&&);
	[[nodiscard]] AnimationRef GetAnimation(const UUIDType&);
	void UnloadAnimation(const UUIDType&, bool removeAllReferences = false);
	void RenameAnimation(const UUIDType&, const stringType& newName);
protected:
	AnimationAssetManager() = default;
private:
	animationContainerType m_animations;	
	loadedAnimationsRefType m_loadedAnimations;
	LockData m_lockData;
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
