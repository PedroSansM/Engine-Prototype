#pragma once

#include "DommusCore.h"

#include <string>
#include <vector>



namespace DEditor
{

class EditorAnimationStateMachine
{
public:
	using animationStateMachineType = DCore::AnimationStateMachine;
	using stateIdType = animationStateMachineType::stateContainerType::idType;
	using stateContainerType = animationStateMachineType::stateContainerType;
	using dVec2 = DCore::DVec2;
	using stateRefType = DCore::AnimationStateMachine::stateRefType;
	using stateConstRefType = DCore::AnimationStateMachine::stateConstRefType;
	using animationRefType = DCore::AnimationRef;
	using createStateResult = DCore::AnimationStateMachine::CreateStateResult;
	using renameStateResult = DCore::AnimationStateMachine::RenameStateResult;
	using createParameterResult = DCore::AnimationStateMachine::CreateParameterResult;
	using renameParameterResult = DCore::AnimationStateMachine::RenameParameterResult;
	using createConditionResult = DCore::AnimationStateMachine::CreateConditionResult;
	using parameterType = DCore::ParameterType;
	using createTransitionResult = DCore::AnimationStateMachine::CreateTransitionResult;
	using transitionConstRefType = DCore::transitionConstRefType;
	using conditionConstRefType = DCore::conditionConstRefType;
	using numericConditionType = DCore::NumericConditionType;
	using uuidType = DCore::UUIDType;
	using entityRefType = DCore::EntityRef;
	using positionContainerType = std::vector<dVec2>;
	using stringType = std::string;
public:
	EditorAnimationStateMachine(const uuidType&);
	EditorAnimationStateMachine(EditorAnimationStateMachine&&) noexcept;
	~EditorAnimationStateMachine() = default;
public:
	createStateResult CreateState(const stringType& stateName, const dVec2& statePosition, stateConstRefType* outState = nullptr);
	stateConstRefType TryGetStateAtIndex(size_t) const;
	stateConstRefType TryGetStateWithName(const stringType& stateName) const;
	renameStateResult SetNameOfState(size_t stateIndex, const stringType&);
	void SetStatePosition(size_t stateIndex, const dVec2&);
	void SetStateAnimation(size_t stateIndex, animationRefType);
	void DeleteState(size_t stateIndex);
	createParameterResult CreateParameter(parameterType, const stringType& parameterName);
	createTransitionResult CreateTransition(size_t fromStateIndex, size_t toStateIndex, transitionConstRefType* outTransition = nullptr);
	transitionConstRefType TryGetTransitionAtIndex(size_t fromStateIndex, size_t transitionIndex) const;
	void DeleteTransition(size_t fromStateIndex, size_t transitionIndex);
	createConditionResult CreateCondition(size_t fromStateIndex, size_t transitionIndex, conditionConstRefType* outCondition = nullptr);
	void SetConditionParameter(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, parameterType, size_t parameterIndex);
	void SetNumericCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, numericConditionType);
	void DeleteCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex);
	void AttachTo(entityRefType);
	void Setup();
	void Tick(float deltaTime);
public:
	EditorAnimationStateMachine& operator=(EditorAnimationStateMachine&&) noexcept;
public:
	const uuidType& GetUUID() const
	{
		return m_animationStateMachine.GetUUID();
	}

	const dVec2& GetPositionOfStateAtIndex(stateIdType stateId) const
	{
		DASSERT_E(stateId < m_statePositions.size());
		return m_statePositions[stateId];
	}

	size_t GetNumberOfStates() const
	{
		return m_animationStateMachine.GetNumberOfStates();
	}

	size_t GetInitialStateIndex() const
	{
		return m_animationStateMachine.GetInitialStateIndex();
	}

	void SetInitialStateIndex(size_t index)
	{
		m_animationStateMachine.SetInitialStateIndex(index);
	}

	animationStateMachineType& GetCoreAnimationStateMachine()
	{
		return m_animationStateMachine;
	}
#ifdef EDITOR
	const stringType& GetName() const
	{
		return m_animationStateMachine.GetName();
	}

	void SetName(const stringType& name)
	{
		m_animationStateMachine.SetName(name);	
	}
#endif
public:
	template <parameterType ParameterType, class ParameterRefType>
	createParameterResult CreateParameter(const stringType& parameterName, ParameterRefType& outParameter)
	{
		return m_animationStateMachine.CreateParameter<ParameterType>(parameterName, outParameter);
	}

	template <parameterType ParameterType>
	decltype(auto) TryGetParameterAtIndex(size_t index) const
	{
		return m_animationStateMachine.TryGetParameterAtIndex<ParameterType>(index);
	}

	template <parameterType ParameterType>
	decltype(auto) TryGetParameterWithName(const stringType& parameterName) const
	{
		return m_animationStateMachine.TryGetParameterWithName<ParameterType>(parameterName);
	}

	template <parameterType ParameterType>
	renameParameterResult RenameParameter(size_t parameterIndex, const stringType& parameterName)
	{
		return m_animationStateMachine.RenameParameter<ParameterType>(parameterIndex, parameterName);
	}

	template <parameterType ParameterT, class Func>
	void IterateOnParameters(Func function)
	{
		m_animationStateMachine.IterateOnParameters<ParameterT>(function);
	}

	template <parameterType ParameterT, class Func>
	void IterateOnParameters(Func function) const
	{
		m_animationStateMachine.IterateOnParameters<ParameterT>(function);
	}

	template <class ParameterRefType>
	void DeleteParameter(ParameterRefType parameter)
	{
		m_animationStateMachine.DeleteParameter(parameter);
	}

	template <parameterType ParameterType, class ParameterValueType>
	void SetParameterValue(size_t parameterIndex, ParameterValueType value)
	{
		m_animationStateMachine.SetParameterValue<ParameterType>(parameterIndex, value);
	}

	template <class Func>
	void IterateOnStates(Func callback)
	{
		m_animationStateMachine.IterateOnStates(callback);
	}

	template <class Func>
	void IterateOnStates(Func callback) const
	{
		m_animationStateMachine.IterateOnStates(callback);
	}

	template <class Func>
	void IterateOnTransitionsOfStateAtIndex(size_t stateIndex, Func callback)
	{
		m_animationStateMachine.IterateOnTransitionsOfStateAtIndex(stateIndex, callback);
	}

	template <class Func>
	void IterateOnConditions(size_t fromStateIndex, size_t transitionIndex, Func function)
	{
		m_animationStateMachine.IterateOnConditions(fromStateIndex, transitionIndex, function);
	}

	template <parameterType ParameterT, class ValueType>
	void SetValueOfCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, ValueType value)
	{
		m_animationStateMachine.SetValueOfCondition<ParameterT>(fromStateIndex, transitionIndex, conditionIndex, value);
	}

	template <class ParameterRefType>
	ParameterRefType MakeInvalidConstRefToParameter() const
	{
		return m_animationStateMachine.MakeInvalidConstRefToParameter<ParameterRefType>();
	}
private:
	animationStateMachineType m_animationStateMachine;
	positionContainerType m_statePositions;
};

}
