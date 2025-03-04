#include "EditorAnimationStateMachine.h"



namespace DEditor
{

EditorAnimationStateMachine::EditorAnimationStateMachine(const uuidType& uuid)
	:
	m_animationStateMachine(uuid)
{}

EditorAnimationStateMachine::EditorAnimationStateMachine(EditorAnimationStateMachine&& other) noexcept
	:
	m_animationStateMachine(std::move(other.m_animationStateMachine)),
	m_statePositions(std::move(other.m_statePositions))
{}

EditorAnimationStateMachine::createStateResult EditorAnimationStateMachine::CreateState(const stringType& stateName, const dVec2& statePosition, stateConstRefType* outState)
{
	stateConstRefType stateRef;
	switch (m_animationStateMachine.CreateState(stateName, &stateRef))
	{
	case createStateResult::Ok:
	{
		stateIdType stateIndex(stateRef.GetIndex());
		if (stateIndex >= m_statePositions.size())
		{
			m_statePositions.resize(stateIndex + 1);
		}
		m_statePositions[stateIndex] = statePosition;
		if (outState != nullptr)
		{
			*outState = stateRef;
		}
		return createStateResult::Ok;
	}
	case createStateResult::StateNameEmpty:
		return createStateResult::StateNameEmpty;
	case createStateResult::StateWithSameName:
		return createStateResult::StateWithSameName;
	}
	return createStateResult::Ok;
}

EditorAnimationStateMachine::stateConstRefType EditorAnimationStateMachine::TryGetStateAtIndex(size_t stateIndex) const
{
	return m_animationStateMachine.TryGetStateAtIndex(stateIndex);
}

EditorAnimationStateMachine::stateConstRefType EditorAnimationStateMachine::TryGetStateWithName(const stringType &stateName) const
{
	return m_animationStateMachine.TryGetStateWithName(stateName);
}

EditorAnimationStateMachine::renameStateResult EditorAnimationStateMachine::SetNameOfState(size_t stateIndex, const stringType& name)
{
	return m_animationStateMachine.SetNameOfState(stateIndex, name);
}

void EditorAnimationStateMachine::SetStatePosition(size_t stateIndex, const dVec2& position)
{
	DASSERT_E(TryGetStateAtIndex(stateIndex).IsValid());
	m_statePositions[stateIndex] = position;
}

void EditorAnimationStateMachine::SetStateAnimation(size_t stateIndex, animationRefType animation)
{
	m_animationStateMachine.SetStateAnimation(stateIndex, animation);
}

void EditorAnimationStateMachine::DeleteState(size_t stateIndex)
{
	m_animationStateMachine.DeleteState(stateIndex);
}

EditorAnimationStateMachine::createParameterResult EditorAnimationStateMachine::CreateParameter(parameterType paraameterType, const stringType& parameterName)
{
	return m_animationStateMachine.CreateParameter(paraameterType, parameterName);
}

EditorAnimationStateMachine::createTransitionResult EditorAnimationStateMachine::CreateTransition(size_t fromStateIndex, size_t toStateIndex, transitionConstRefType* outTransition)
{
	return m_animationStateMachine.CreateTransition(fromStateIndex, toStateIndex, outTransition);
}

EditorAnimationStateMachine::transitionConstRefType EditorAnimationStateMachine::TryGetTransitionAtIndex(size_t fromStateIndex, size_t transitionIndex) const
{
	return m_animationStateMachine.TryGetTransitionAtIndex(fromStateIndex, transitionIndex);
}

void EditorAnimationStateMachine::DeleteTransition(size_t fromStateIndex, size_t transitionIndex)
{
	m_animationStateMachine.DeleteTransition(fromStateIndex, transitionIndex);
}

EditorAnimationStateMachine::createConditionResult EditorAnimationStateMachine::CreateCondition(size_t fromStateIndex, size_t transitionIndex, conditionConstRefType* outCondition)
{
	return m_animationStateMachine.CreateCondition(fromStateIndex, transitionIndex, outCondition);
}

void EditorAnimationStateMachine::SetConditionParameter(
	size_t fromStateIndex, 
	size_t transitionIndex, 
	size_t conditionIndex, 
	parameterType parameterType, 
	size_t parameterIndex)
{
	m_animationStateMachine.SetConditionParameter(fromStateIndex, transitionIndex, conditionIndex, parameterType, parameterIndex);
}

void EditorAnimationStateMachine::SetNumericCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, numericConditionType numericCondition)
{
	m_animationStateMachine.SetNumericCondition(fromStateIndex, transitionIndex, conditionIndex, numericCondition);
}

void EditorAnimationStateMachine::DeleteCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex)
{
	m_animationStateMachine.DeleteCondition(fromStateIndex, transitionIndex, conditionIndex);
}

void EditorAnimationStateMachine::AttachTo(entityRefType entity)
{
	m_animationStateMachine.AttachTo(entity);
}

void EditorAnimationStateMachine::Setup()
{
	m_animationStateMachine.Setup();
}

void EditorAnimationStateMachine::Tick(float deltaTime)
{
	DCore::ReadWriteLockGuard sceneGuard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	DCore::ReadWriteLockGuard animationGuard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
	DCore::ReadWriteLockGuard asmGuard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationStateMachineAssetManager*>(&DCore::AssetManager::Get()));
	m_animationStateMachine.Tick(deltaTime, [](DCore::EntityRef, size_t){});
}

EditorAnimationStateMachine& EditorAnimationStateMachine::operator=(EditorAnimationStateMachine&& other) noexcept
{
	m_animationStateMachine = std::move(other.m_animationStateMachine);
	m_statePositions = std::move(m_statePositions);
	return *this;
}

}
