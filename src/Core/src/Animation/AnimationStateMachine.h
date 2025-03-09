#pragma once

#include "Parameter.h"
#include "ReciclingVector.h"
#include "State.h"
#include "AttributeRef.h"
#include "AssetManagerTypes.h"

#include <cfloat>
#include <vector>
#include <string>



namespace DCore
{

class AnimationStateMachine
{
public:
	using stringType = std::string;
	using stateContainerType = ReciclingVector<State>;
	using stateRefType = stateContainerType::Ref;
	using stateConstRefType = stateContainerType::ConstRef;
public:
	enum class RenameStateResult
	{
		Ok,
		StateNameEmpty,
		StateWithSameName
	};

	enum class CreateStateResult
	{
		Ok,
		StateNameEmpty,
		StateWithSameName
	};

	enum class CreateParameterResult
	{
		Ok,
		ParameterNameEmpty,
		ParameterWithSameName
	};

	enum class RenameParameterResult
	{
		Ok,
		ParameterNameEmpty,
		ParameterWithSameName	
	};

	enum class CreateTransitionResult
	{
		Ok,
		AlreadyExists
	};
	
	enum class CreateConditionResult
	{
		Ok,
		NoParameterCreated
	};
public:
	AnimationStateMachine() = default;
	AnimationStateMachine(const UUIDType&);
	AnimationStateMachine(const AnimationStateMachine&);
	AnimationStateMachine(AnimationStateMachine&&) noexcept;
	~AnimationStateMachine() = default;
public:
	CreateStateResult CreateState(const stringType& stateName, stateConstRefType* outState = nullptr);
	stateConstRefType TryGetStateAtIndex(size_t) const;
	stateConstRefType TryGetStateWithName(const stringType& stateName) const;
	RenameStateResult SetNameOfState(size_t stateIndex, const stringType&);
	void SetStateAnimation(size_t stateIndex, AnimationRef);
	void DeleteState(size_t stateIndex);
	CreateParameterResult CreateParameter(ParameterType, const stringType& parameterName);
	CreateTransitionResult CreateTransition(size_t fromStateIndex, size_t toStateIndex, transitionConstRefType* outTransition = nullptr);
	transitionConstRefType TryGetTransitionAtIndex(size_t fromStateIndex, size_t transitionIndex) const;
	void DeleteTransition(size_t fromStateIndex, size_t transitionIndex);
	CreateConditionResult CreateCondition(size_t fromStateIndex, size_t transitionIndex, conditionConstRefType* outCondition = nullptr);
	void SetConditionParameter(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, ParameterType, size_t parameterIndex);
	void SetNumericCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, NumericConditionType);
	void DeleteCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex);
	void AttachTo(EntityRef entity);
public:
	AnimationStateMachine& operator=(AnimationStateMachine&&) noexcept;
public:
	const UUIDType& GetUUID() const
	{
		return m_uuid;
	}

	size_t GetNumberOfStates() const
	{
		return m_states.Size();
	}

	size_t GetInitialStateIndex() const
	{
		return m_initialStateIndex;
	}

	void SetInitialStateIndex(size_t index)
	{
		m_initialStateIndex = index;
	}

	void Setup()
	{
		m_currentStateIndex = m_initialStateIndex;
	}
#ifdef EDITOR
	const stringType& GetName() const
	{
		return m_name;
	}

	void SetName(const stringType& name)
	{
		m_name = name;
	}
#endif
public:
	template <ParameterType ParameterT, class ParameterRefType>
	CreateParameterResult CreateParameter(const stringType& parameterName, ParameterRefType& outParameter)
	{
		static_assert(ParameterRefType::isConstRef, "Only const refs can be made to animation state machine parameters.");
		static_assert(ParameterT == ParameterRefType::valueType::parameterType, "Inconsistency with parameter type and parameter ref.");
		if (parameterName.empty())
		{
			return CreateParameterResult::ParameterNameEmpty;
		}
		if (!IsParameterNameUnique(parameterName))
		{
			return CreateParameterResult::ParameterWithSameName;
		}
		if constexpr (ParameterT == ParameterType::Integer)
		{
			outParameter = m_integerParameters.PushBackConst(Parameter(parameterName, 0));
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			outParameter = m_floatParameters.PushBackConst(Parameter(parameterName, 0.0f));
		}
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			outParameter = m_logicParameters.PushBackConst(Parameter(parameterName, LogicParameter{false}));
		}
		else
		{
			outParameter = m_triggerParameters.PushBackConst(Parameter(parameterName, TriggerParameter{false}));
		}	
		return CreateParameterResult::Ok;
	}

	template <ParameterType ParameterT>
	auto TryGetParameterAtIndex(size_t index) const
	{
		if constexpr (ParameterT == ParameterType::Integer)
		{
			return m_integerParameters.GetRefFromIndex(index);
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			return m_floatParameters.GetRefFromIndex(index); 
		}
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			return m_logicParameters.GetRefFromIndex(index);
		}
		else
		{
			return m_triggerParameters.GetRefFromIndex(index);
		}
	}

	template <ParameterType ParameterT>
	auto TryGetParameterWithName(const stringType& parameterName) const
	{
		if constexpr (ParameterT == ParameterType::Integer)
		{
			integerParameterConstRefType toReturn;
			m_integerParameters.IterateConstRef
			(
				[&](integerParameterConstRefType parameter) -> bool
				{
					if (parameter->GetName() == parameterName)
					{
						toReturn = parameter;
						return true;
					}
					return false;
				}
			);
			return toReturn;
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			floatParameterConstRefType toReturn;
			m_floatParameters.IterateConstRef
			(
				[&](floatParameterConstRefType parameter) -> bool
				{
					if (parameter->GetName() == parameterName)
					{
						toReturn = parameter;
						return true;
					}
					return false;
				}
			);
			return toReturn;
		}
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			logicParameterConstRefType toReturn;
			m_logicParameters.IterateConstRef
			(
				[&](logicParameterConstRefType parameter) -> bool
				{
					if (parameter->GetName() == parameterName)
					{
						toReturn = parameter;
						return true;
					}
					return false;
				}
			);
			return toReturn;
		}
		else
		{
			triggerParameterConstRefType toReturn;
			m_triggerParameters.IterateConstRef
			(
				[&](triggerParameterConstRefType parameter) -> bool
				{
					if (parameter->GetName() == parameterName)
					{
						toReturn = parameter;
						return true;
					}
					return false;
				}
			);
			return toReturn;
		}
	}

	template <ParameterType ParameterT, class ParameterValueType>
	void SetParameterValue(size_t parameterIndex, ParameterValueType value)
	{
		static_assert
		(
			(ParameterT == ParameterType::Integer && std::is_same_v<ParameterValueType, int>) ||
			(ParameterT == ParameterType::Float && std::is_same_v<ParameterValueType, float>) || 
			(ParameterT == ParameterType::Logic && std::is_same_v<ParameterValueType, LogicParameter>) ||
			(ParameterT == ParameterType::Trigger && std::is_same_v<ParameterValueType, TriggerParameter>),
			"Parameter type and parameter value don't match."
		);
		if constexpr (ParameterT == ParameterType::Integer)
		{
			integerParameterRefType parameter(m_integerParameters.GetRefFromIndex(parameterIndex));
			if (!parameter.IsValid())
			{
				return;
			}
			parameter->SetValue(value);
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			floatParameterRefType parameter(m_floatParameters.GetRefFromIndex(parameterIndex));
			if (!parameter.IsValid())
			{
				return;
			}
			parameter->SetValue(value);
		}
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			logicParameterRefType parameter(m_logicParameters.GetRefFromIndex(parameterIndex));
			if (!parameter.IsValid())
			{
				return;
			}
			parameter->SetValue(value);
		}
		else
		{
			triggerParameterRefType parameter(m_triggerParameters.GetRefFromIndex(parameterIndex));
			if (!parameter.IsValid())
			{
				return;
			}
			parameter->SetValue(value);
		}
	}

	template <ParameterType ParameterT>
	RenameParameterResult RenameParameter(size_t parameterIndex, const stringType& parameterName)
	{
		if (parameterName.empty())
		{
			return RenameParameterResult::ParameterNameEmpty;
		}
		if (!IsParameterNameUnique(parameterName))
		{
			return RenameParameterResult::ParameterWithSameName;
		}
		auto parameter(TryGetParameterAtIndex<ParameterT>(parameterIndex));
		parameter->SetName(parameterName);
		return RenameParameterResult::Ok;
	}

	template <ParameterType ParameterT, class Func>
	void IterateOnParameters(Func function)
	{
		if constexpr (ParameterT == ParameterType::Integer)
		{
			m_integerParameters.IterateConstRef(function);
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			m_floatParameters.IterateConstRef(function);
		}	
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			m_logicParameters.IterateConstRef(function);
		}
		else 
		{
			m_triggerParameters.IterateConstRef(function);
		}
	}

	template <ParameterType ParameterT, class Func>
	void IterateOnParameters(Func function) const
	{
		if constexpr (ParameterT == ParameterType::Integer)
		{
			m_integerParameters.IterateConstRef(function);
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			m_floatParameters.IterateConstRef(function);
		}	
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			m_logicParameters.IterateConstRef(function);
		}
		else 
		{
			m_triggerParameters.IterateConstRef(function);
		}
	}

	template <class ParameterRefType>
	void DeleteParameter(ParameterRefType parameter)
	{
		static_assert
		(
			std::is_same_v<integerParameterConstRefType, ParameterRefType> ||
			std::is_same_v<floatParameterConstRefType, ParameterRefType> ||		
			std::is_same_v<logicParameterConstRefType, ParameterRefType> ||
			std::is_same_v<triggerParameterConstRefType, ParameterRefType>,
			"Invalid ref."
		);
		// TODO. Handle the case where there is conditions that reference the parameter to be deleted.
		if constexpr (std::is_same_v<integerParameterConstRefType, ParameterRefType>)
		{
			m_integerParameters.Remove(parameter);
		}
		else if constexpr (std::is_same_v<floatParameterConstRefType, ParameterRefType>)
		{
			m_floatParameters.Remove(parameter);
		}
		else if constexpr (std::is_same_v<logicParameterConstRefType, ParameterRefType>)
		{
			m_logicParameters.Remove(parameter);
		}
		else 
		{
			m_triggerParameters.Remove(parameter);
		}
	}

	template <class Func>
	void IterateOnStates(Func callback)
	{
		m_states.IterateConstRef(callback);
	}
	
	template <class Func>
	void IterateOnStates(Func callback) const
	{
		m_states.IterateConstRef(callback);
	}

	template <class Func>
	void IterateOnTransitionsOfStateAtIndex(size_t index, Func function)
	{
		stateRefType state(TryGetStateAtIndex(index));
		state->IterateOnTransitions(function);
	}

	template <class Func>
	void IterateOnConditions(size_t fromStateIndex, size_t transitionIndex, Func function)
	{
		stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
		transitionRefType transition(fromState->TryGetTransitionAtIndex(transitionIndex));
		DASSERT_E(fromState.IsValid() && transition.IsValid());
		transition->IterateOnConditions(function);
	}

	template <ParameterType ParameterT, class ValueType>
	void SetValueOfCondition(size_t fromStateIndex, size_t transitionIndex, size_t conditionIndex, ValueType value)
	{
		stateRefType fromState(m_states.GetRefFromIndex(fromStateIndex));
		DASSERT_E(fromState.IsValid());
		transitionRefType transition(fromState->TryGetTransitionAtIndex(transitionIndex));
		DASSERT_E(transition.IsValid());
		conditionRefType condition(transition->TryGetConditionAtIndex(conditionIndex));
		DASSERT_E(condition.IsValid());
		condition->SetValue<ParameterT>(value);
	}

	template <class ParameterRefType>
	ParameterRefType MakeInvalidConstRefToParameter() const
	{
		static_assert(std::is_same<integerParameterConstRefType, ParameterRefType>::value || 
			std::is_same<floatParameterConstRefType, ParameterRefType>::value || 
			std::is_same<logicParameterConstRefType, ParameterRefType>::value || 
			std::is_same<triggerParameterConstRefType, ParameterRefType>::value, 
			"Unsupported parameter ref type.");	
		if constexpr (std::is_same_v<integerParameterConstRefType, ParameterRefType>)
		{
			return decltype(m_integerParameters)::ConstRef();
		}
		else if constexpr (std::is_same_v<floatParameterConstRefType, ParameterRefType>)
		{
			return decltype(m_floatParameters)::ConstRef();
		}
		else if constexpr (std::is_same_v<logicParameterConstRefType, ParameterRefType>)
		{
			return decltype(m_logicParameters)::ConstRef();
		}
		else
		{
			return decltype(m_triggerParameters)::ConstRef();
		}
	}

	template <ParameterType ParameterT>
	bool TryGetParameterIndexWithName(const stringType& name, size_t& out)
	{
		out = 0;
		const auto tryGetParameterIndex
		(
			[&](const auto& parameterContainer) -> bool
			{
				bool returnValue(false);
				parameterContainer.Iterate
				(
					[&](auto parameter) -> bool
					{
						if (parameter->GetName() == name)
						{
							returnValue = true;
							out = parameter.GetIndex();
							return true;
						}
						return false;
					}
				);
				return returnValue;
			}
		);
		if constexpr (ParameterT == ParameterType::Integer)
		{
			return tryGetParameterIndex(m_integerParameters);
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			return tryGetParameterIndex(m_floatParameters);
		}
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			return tryGetParameterIndex(m_logicParameters);
		}
		else
		{
			return tryGetParameterIndex(m_triggerParameters);
		}
	}

	template <class Func>
	void Tick(float deltaTime, Func metachannelsCallback)
	{
		if (m_states.Empty())
		{
			return;
		}
		stateRefType state(m_states.GetRefFromIndex(m_currentStateIndex));
		constexpr size_t maxNumberOfTransitionAttempts(10);
		size_t currentNumberOfTransitionsAttempts(0);
		while (true)
		{
			if (state->TryMakeTransition(m_currentStateIndex))
			{
				m_currentTime = 0.0f;
				state = m_states.GetRefFromIndex(m_currentStateIndex);
			}
			currentNumberOfTransitionsAttempts++;
			if (currentNumberOfTransitionsAttempts >= maxNumberOfTransitionAttempts)
			{
				break;
			}
		}
		if (state->GetAnimation().IsValid())
		{
			state->Tick(m_currentTime, m_currentTime + deltaTime, metachannelsCallback);
			m_currentTime += deltaTime;
			if (m_currentTime >= state->GetAnimation().GetDuration())
			{
				m_currentTime = 0.0f;
			}
		}
	}
private:
	UUIDType m_uuid;
	stateContainerType m_states;
	size_t m_currentStateIndex;
	size_t m_initialStateIndex;
	integerParameterContainerType m_integerParameters;
	floatParameterContainerType m_floatParameters;
	logicParameterContainerType m_logicParameters;
	triggerParameterContainerType m_triggerParameters;
	float m_currentTime;
#ifdef EDITOR
	stringType m_name;
#endif
private:
	stateRefType TryGetStateAtIndex(size_t stateIndex);
	bool IsParameterNameUnique(const stringType&) const;
private:
	template <class ContainerType>
	CreateParameterResult CreateParameter(
		const stringType& parameterName, 
		ContainerType& parameterContainer, 
		typename ContainerType::valueType&& defaultValue)
	{
		if (parameterName.empty())
		{
			return CreateParameterResult::ParameterNameEmpty;
		}
		if (!IsParameterNameUnique(parameterName))
		{
			return CreateParameterResult::ParameterWithSameName;
		}
		parameterContainer.PushBack(std::move(defaultValue));
		return CreateParameterResult::Ok;
	}

	template <ParameterType ParameterT>
	decltype(auto) TryGetParameterAtIndex(size_t index)
	{
		if constexpr (ParameterT == ParameterType::Integer)
		{
			return m_integerParameters.GetRefFromIndex(index);
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			return m_floatParameters.GetRefFromIndex(index); 
		}
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			return m_logicParameters.GetRefFromIndex(index);
		}
		else
		{
			return m_triggerParameters.GetRefFromIndex(index);
		}
	}

	template <class ContainerType>
	bool IsParameterNameUnique(const stringType& name, const ContainerType& parameterContainer) const
	{
		bool isUnique(true);
		parameterContainer.Iterate
		(
			[&](typename ContainerType::ConstRef parameterRef) -> bool
			{
				if (name == parameterRef->GetName())
				{
					isUnique = false;
					return true;
				}
				return false;
			}
		);
		return isUnique;
	}
};

using InternalAnimationStateMachineRefType = typename AssetContainerType<AnimationStateMachine>::Ref;

class AnimationStateMachineRef
{
public:
	using stringType = std::string;
public:
	AnimationStateMachineRef();
	AnimationStateMachineRef(InternalAnimationStateMachineRefType, LockData&);
	AnimationStateMachineRef(const AnimationStateMachineRef&);
	~AnimationStateMachineRef() = default;
public:
	bool IsValid();
	void Unload();
	UUIDType GetUUID();
	void AttachTo(EntityRef);
#ifdef EDITOR
	stringType GetName();
	void SetName(const stringType& name);
#endif
public:
	AnimationStateMachineRef& operator=(const AnimationStateMachineRef&);
public:
	InternalAnimationStateMachineRefType GetInternalRef() const
	{
		return m_ref;
	}

	void Setup()
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		m_ref->GetAsset().Setup();
	}
public:
	template <class Func>
	void Tick(float deltaTime, Func metachannelsCallback)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		DASSERT_E(IsValid());
		m_ref->GetAsset().Tick(deltaTime, metachannelsCallback);
	}

	template <ParameterType ParameterT>
	bool TryGetParameterIndexWithName(const stringType& name, size_t& out)
	{
		if (!IsValid())
		{
			return false;
		}
		return m_ref->GetAsset().TryGetParameterIndexWithName<ParameterT>(name, out);
	}

	template <ParameterType ParameterT, class ParameterValueType>
	void SetParameterValue(size_t parameterIndex, ParameterValueType value)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, *m_lockData);
		if (!IsValid())
		{
			return;
		}
		m_ref->GetAsset().SetParameterValue<ParameterT>(parameterIndex, value);
	}
private:
	InternalAnimationStateMachineRefType m_ref;
	LockData* m_lockData;
};

}
