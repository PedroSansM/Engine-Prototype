#pragma once

#include "ReciclingVector.h"
#include "AnimationStateMachine.h"
#include "ReadWriteLockGuard.h"



namespace DCore
{

class AnimationStateMachineAssetManager
{
	friend class ReadWriteLockGuard;
public:
	using animationStateMachineContainerType = AssetContainerType<AnimationStateMachine>;
public:
	AnimationStateMachineAssetManager(const AnimationStateMachineAssetManager&) = delete;
	AnimationStateMachineAssetManager(AnimationStateMachineAssetManager&&) = delete;
	~AnimationStateMachineAssetManager();
public:
	AnimationStateMachineRef AddAnimationStateMachine(AnimationStateMachine&&);
	void RemoveAnimationStateMachine(AnimationStateMachineRef);
	void RemoveAllAnimationStateMachineInstancesWithUUID(const UUIDType&);
public:
	template <class Func>
	void IterateOnAnimationStateMachines(Func function)
	{
		m_animationStateMachines.Iterate
		(
			[&](animationStateMachineContainerType::Ref animationStateMachine) -> bool
			{
				return std::invoke(function, animationStateMachine);
			}
		);
	}
protected:
	AnimationStateMachineAssetManager() = default;
private:
	animationStateMachineContainerType m_animationStateMachines;
	LockData m_lockData;
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
