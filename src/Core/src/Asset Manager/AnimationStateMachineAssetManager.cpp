#include "AnimationStateMachineAssetManager.h"



namespace DCore
{

AnimationStateMachineAssetManager::~AnimationStateMachineAssetManager()
{
	m_animationStateMachines.Clear();
}

AnimationStateMachineRef AnimationStateMachineAssetManager::AddAnimationStateMachine(AnimationStateMachine&& animationStateMachine)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	return AnimationStateMachineRef(m_animationStateMachines.PushBack(animationStateMachine.GetUUID(), std::move(animationStateMachine)), m_lockData);
}

void AnimationStateMachineAssetManager::RemoveAnimationStateMachine(AnimationStateMachineRef ref)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	m_animationStateMachines.Remove(ref.GetInternalRef());
}

void AnimationStateMachineAssetManager::RemoveAllAnimationStateMachineInstancesWithUUID(const UUIDType& uuid)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	m_animationStateMachines.Iterate
	(
		[&](animationStateMachineContainerType::ConstRef animationStateMachine) -> bool
		{
			if (animationStateMachine->GetUUID() == uuid)
			{
				m_animationStateMachines.Remove(animationStateMachine);
			}
			return false;
		}
	);
}

}
