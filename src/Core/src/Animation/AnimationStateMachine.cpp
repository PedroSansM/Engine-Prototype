#include "AnimationStateMachine.h"
#include "DCoreAssert.h"
#include "ReadWriteLockGuard.h"
#include "SerializationTypes.h"
#include "Transition.h"
#include "State.h"
#include "AssetManager.h"



namespace DCore
{

// AnimationStateMachine
AnimationStateMachine::AnimationStateMachine(const UUIDType& uuid)
	:
	m_uuid(uuid),
	m_currentStateIndex(0),
	m_initialStateIndex(0),
	m_currentTime(0.0f)
{}

AnimationStateMachine::AnimationStateMachine(const AnimationStateMachine& other)
	:
	m_uuid(other.m_uuid),
	m_currentStateIndex(other.m_currentStateIndex),
	m_initialStateIndex(other.m_initialStateIndex),
	m_integerParameters(other.m_integerParameters),
	m_floatParameters(other.m_floatParameters),
	m_logicParameters(other.m_logicParameters),
	m_triggerParameters(other.m_triggerParameters),
	m_currentTime(other.m_currentTime)
#ifdef EDITOR
	, m_name(other.m_name)
#endif
{
	other.m_states.Iterate
	(
		[&](stateConstRefType state) -> bool
		{
			m_states.PushBack(State(*state.Data(), m_integerParameters, m_floatParameters, m_logicParameters, m_triggerParameters));
			return false;
		}
	);
}

AnimationStateMachine::AnimationStateMachine(AnimationStateMachine&& other) noexcept
	:
	m_uuid(other.m_uuid),
	m_currentStateIndex(other.m_currentStateIndex),
	m_initialStateIndex(other.m_initialStateIndex),
	m_integerParameters(std::move(other.m_integerParameters)),
	m_floatParameters(std::move(other.m_floatParameters)),
	m_logicParameters(std::move(other.m_logicParameters)),
	m_triggerParameters(std::move(other.m_triggerParameters)),
	m_currentTime(other.m_currentTime)
#ifdef EDITOR
	, m_name(std::move(other.m_name))
#endif
{
	other.m_states.Iterate
	(
		[&](stateRefType state) -> bool
		{
			m_states.PushBack(State(std::move(*state.Data()), m_integerParameters, m_floatParameters, m_logicParameters, m_triggerParameters));
			return false;
		}
	);
}
 
AnimationStateMachine::CreateStateResult AnimationStateMachine::CreateState(const stringType& stateName, stateConstRefType* outState)
{
	if (stateName.empty())
	{
		return CreateStateResult::StateNameEmpty;
	}
	CreateStateResult result(CreateStateResult::Ok);
	m_states.Iterate
	(
		[&](stateRefType state) -> bool
		{
			if (state->GetName() == stateName)
			{
				result = CreateStateResult::StateWithSameName;
				return true;
			}
			return false;
		}
	);
	if (result == CreateStateResult::Ok)
	{
		stateConstRefType state(m_states.PushBackConst(stateName));
		if (outState != nullptr)
		{
			*outState = state;
		}
	}
	return result;
}

AnimationStateMachine::stateConstRefType AnimationStateMachine::TryGetStateAtIndex(size_t stateIndex) const
{
	return m_states.GetRefFromIndex(stateIndex);
}

AnimationStateMachine::stateConstRefType AnimationStateMachine::TryGetStateWithName(const stringType& stateName) const
{
	stateConstRefType toReturn;
	m_states.Iterate
	(
		[&](stateConstRefType state) -> bool
		{
			if (state->GetName() == stateName)
			{
				toReturn = state;
				return true;
			}
			return false;
		}
	);
	return toReturn;
}

AnimationStateMachine::RenameStateResult AnimationStateMachine::SetNameOfState(size_t stateIndex, const stringType& name)
{
	stateRefType state(m_states.GetRefFromIndex(stateIndex));
	DASSERT_E(state.IsValid());
	if (name.empty())
	{
		return RenameStateResult::StateNameEmpty;
	}
	RenameStateResult result(RenameStateResult::Ok);
	m_states.Iterate
	(
		[&](stateRefType ref) -> bool
		{
			if (ref != state && name == ref->GetName())
			{
				result = RenameStateResult::StateWithSameName;
				return true;
			}
			return false;
		}
	);
	if (result == RenameStateResult::Ok)
	{
		state->SetName(name);
	}
	return result;
}

void AnimationStateMachine::SetStateAnimation(size_t stateIndex, AnimationRef animation)
{
	stateRefType state(m_states.GetRefFromIndex(stateIndex));
	DASSERT_E(state.IsValid());
	state->SetAnimation(animation);
}

void AnimationStateMachine::DeleteState(size_t stateIndex)
{
	m_states.RemoveElementAtIndex(stateIndex);
	m_states.Iterate
	(
		[&](stateRefType state) -> bool
		{
			state->DeleteAllTransitionsThatGoToStateAtIndex(stateIndex);
			return false;
		}
	);
}

AnimationStateMachine::CreateParameterResult AnimationStateMachine::CreateParameter(ParameterType typeParameter, const stringType& parameterName)
{
	switch (typeParameter)
	{
	case ParameterType::Integer:
		return CreateParameter(parameterName, m_integerParameters, Parameter(parameterName, 0));
	case ParameterType::Float:
		return CreateParameter(parameterName, m_floatParameters, Parameter(parameterName, 0.0f));
	case ParameterType::Logic:
		return CreateParameter(parameterName, m_logicParameters, Parameter(parameterName, LogicParameter{false}));
	case ParameterType::Trigger:
		return CreateParameter(parameterName, m_triggerParameters, Parameter(parameterName, TriggerParameter{false}));
	}
	DASSERT_E(false);
	return CreateParameterResult::Ok;
}

AnimationStateMachine::CreateTransitionResult AnimationStateMachine::CreateTransition(size_t fromStateIndex, size_t toStateIndex, transitionConstRefType* outTransition)
{
	using stateConstRefType = stateContainerType::ConstRef;
	stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
	stateConstRefType toState(m_states.GetRefFromIndex(toStateIndex));
	DASSERT_E(fromState.IsValid() && toState.IsValid());
	if (fromState->TransitionExists(toStateIndex))
	{
		return CreateTransitionResult::AlreadyExists;
	}
	fromState->AddTransition(fromStateIndex, toStateIndex, outTransition);
	return CreateTransitionResult::Ok;
}

transitionConstRefType AnimationStateMachine::TryGetTransitionAtIndex(size_t fromStateIndex, size_t transitionIndex) const
{
	stateConstRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
	DASSERT_E(fromState.IsValid());
	transitionConstRefType transition(fromState->TryGetTransitionAtIndex(transitionIndex));
	DASSERT_E(transition.IsValid());
	return transition;
}

void AnimationStateMachine::DeleteTransition(size_t fromStateIndex, size_t transitionIndex)
{
	stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
	DASSERT_E(fromState.IsValid());
	fromState->DeleteTransitionAtIndex(transitionIndex);
}

AnimationStateMachine::CreateConditionResult AnimationStateMachine::CreateCondition(size_t fromStateIndex, size_t transitionIndex, conditionConstRefType* outCondition)
{
	if (m_integerParameters.Empty() && 
		m_floatParameters.Empty() && 
		m_logicParameters.Empty() && 
		m_triggerParameters.Empty())
	{
		return CreateConditionResult::NoParameterCreated;
	}
	conditionConstRefType condition;
	stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
	DASSERT_E(fromState.IsValid());
	transitionRefType transition(fromState->TryGetTransitionAtIndex(transitionIndex));
	if (!m_integerParameters.Empty())
	{
		m_integerParameters.IterateConstRef
		(
			[&](integerParameterConstRefType parameter) -> bool
			{
				condition = transition->CreateCondition(ParameterType::Integer, parameter.GetIndex(), m_integerParameters, m_floatParameters, m_logicParameters, m_triggerParameters);
				return true;
			}
		);
	}
	else if (!m_floatParameters.Empty())
	{
		m_floatParameters.IterateConstRef
		(
			[&](floatParameterConstRefType parameter) -> bool
			{
				condition = transition->CreateCondition(ParameterType::Float, parameter.GetIndex(), m_integerParameters, m_floatParameters, m_logicParameters, m_triggerParameters);
				return true;
			}
		);
	}
	else if (!m_logicParameters.Empty())
	{
		m_logicParameters.IterateConstRef
		(
			[&](logicParameterConstRefType parameter) -> bool
			{
				condition = transition->CreateCondition(ParameterType::Logic, parameter.GetIndex(), m_integerParameters, m_floatParameters, m_logicParameters, m_triggerParameters);
				return true;
			}
		);
	}
	else 
	{
		m_triggerParameters.IterateConstRef
		(
			[&](triggerParameterConstRefType parameter) -> bool
			{
				condition = transition->CreateCondition(ParameterType::Trigger, parameter.GetIndex(), m_integerParameters, m_floatParameters, m_logicParameters, m_triggerParameters);
				return true;
			}
		);
	}
	if (outCondition != nullptr)
	{
		*outCondition = condition;
	}
	return CreateConditionResult::Ok;
}	

void AnimationStateMachine::SetConditionParameter(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, ParameterType parameterType, size_t parameterIndex)
{
	stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
	DASSERT_E(fromState.IsValid());
	transitionRefType transition(fromState->TryGetTransitionAtIndex(transitionIndex));
	DASSERT_E(transition.IsValid());
	conditionRefType condition(transition->TryGetConditionAtIndex(conditionIndex));
	DASSERT_E(condition.IsValid());
	condition->SetParameterType(parameterType);
	condition->SetParameterIndex(parameterIndex);
}

void AnimationStateMachine::SetNumericCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, NumericConditionType numericCondition)
{
	stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
	DASSERT_E(fromState.IsValid());
	transitionRefType transition(fromState->TryGetTransitionAtIndex(transitionIndex));
	DASSERT_E(transition.IsValid());
	conditionRefType condition(transition->TryGetConditionAtIndex(conditionIndex));
	DASSERT_E(condition.IsValid());
	condition->SetNumericCondition(numericCondition);
}

void AnimationStateMachine::DeleteCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex)
{
	stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
	DASSERT_E(fromState.IsValid());
	transitionRefType transition(fromState->TryGetTransitionAtIndex(transitionIndex));
	DASSERT_E(transition.IsValid());
	transition->DeleteCondition(conditionIndex);
}

void AnimationStateMachine::AttachTo(EntityRef entity)
{
	m_states.Iterate
	(
		[&](stateRefType state) -> bool
		{
			state->SetEntity(entity);
			return false;
		}
	);
}

AnimationStateMachine& AnimationStateMachine::operator=(AnimationStateMachine&& other) noexcept
{
	m_uuid = other.m_uuid;
	m_currentStateIndex = other.m_currentStateIndex;
	m_initialStateIndex = other.m_initialStateIndex;
	m_integerParameters = std::move(other.m_integerParameters);
	m_floatParameters = std::move(other.m_floatParameters);
	m_logicParameters = std::move(other.m_logicParameters);
	m_triggerParameters = std::move(other.m_triggerParameters);
	m_currentTime = other.m_currentTime;
	other.m_states.Iterate
	(
		[&](stateRefType state) -> bool
		{
			m_states.PushBack(State(std::move(*state.Data()), m_integerParameters, m_floatParameters, m_logicParameters, m_triggerParameters));
			return false;
		}
	);
#ifdef EDITOR
	m_name = std::move(other.m_name);
#endif
	return *this;
}

bool AnimationStateMachine::IsParameterNameUnique(const stringType& name) const
{
	return IsParameterNameUnique(name, m_integerParameters) && IsParameterNameUnique(name, m_floatParameters) && IsParameterNameUnique(name, m_logicParameters) && IsParameterNameUnique(name, m_triggerParameters);
}

AnimationStateMachine::stateRefType AnimationStateMachine::TryGetStateAtIndex(size_t stateIndex)
{
	return m_states.GetRefFromIndex(stateIndex);
}
// End AnimationStateMachine

// AnimationStateMachineRef
AnimationStateMachineRef::AnimationStateMachineRef(InternalAnimationStateMachineRefType ref, LockData& lockData)
	:
	m_ref(ref),
	m_lockData(&lockData)
{}

AnimationStateMachineRef::AnimationStateMachineRef(const AnimationStateMachineRef& other)
	:
	m_ref(other.m_ref),
	m_lockData(other.m_lockData)
{}

bool AnimationStateMachineRef::IsValid()
{
	return m_lockData != nullptr && m_ref.IsValid();
}

void AnimationStateMachineRef::Unload()
{
	if (IsValid())
	{
		AssetManager::Get().RemoveAnimationStateMachine(*this);
	}
}

UUIDType AnimationStateMachineRef::GetUUID()
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetUUID();
}

void AnimationStateMachineRef::AttachTo(EntityRef entity)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().AttachTo(entity);
}

#ifdef EDITOR
AnimationStateMachineRef::stringType AnimationStateMachineRef::GetName()
{
	DASSERT_E(IsValid());
	return m_ref->GetAsset().GetName();
}

void AnimationStateMachineRef::SetName(const stringType& name)
{
	DASSERT_E(IsValid());
	ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
	m_ref->GetAsset().SetName(name);
}
#endif

AnimationStateMachineRef& AnimationStateMachineRef::operator=(const AnimationStateMachineRef& other)
{
	m_ref = other.m_ref;
	m_lockData = other.m_lockData;
	return *this;
}
// End AnimationStateMachineRef

}
