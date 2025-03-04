#pragma once

#include "DCoreAssert.h"
#include "Condition.h"

#include <cstddef>
#include <iterator>
#include <vector>



namespace DCore
{

class Transition
{
public:
	Transition(size_t fromStateIndex, size_t toStateIndex);
	Transition(const Transition&,
			integerParameterContainerType&,
			floatParameterContainerType&,
			logicParameterContainerType&,
			triggerParameterContainerType&);
	Transition(Transition&&) noexcept;
	~Transition() = default;
public:
	conditionRefType CreateCondition(ParameterType conditionParameter,
			size_t parameterIndex, 
			integerParameterContainerType& integerParameters,
			floatParameterContainerType& floatParameters,
			logicParameterContainerType& logicParameters,
			triggerParameterContainerType& triggerParameters);
	bool ToExecute();
	void DeleteCondition(size_t conditionIndex);
public:
	Transition& operator=(Transition&& other) noexcept;
public:
	size_t GetFromStateIndex() const
	{
		return m_fromStateIndex;
	}

	size_t GetToStateIndex() const
	{
		return m_toStateIndex;
	}

	conditionRefType TryGetConditionAtIndex(size_t conditionIndex)
	{
		return m_conditions.GetRefFromIndex(conditionIndex);
	}
public:
	template <class Func>
	void IterateOnConditions(Func function)
	{
		m_conditions.IterateConstRef(function);
	}

	template <class Func>
	void IterateOnConditions(Func function) const
	{
		m_conditions.IterateConstRef(function);
	}
private:
	size_t m_fromStateIndex;
	size_t m_toStateIndex;
	conditionContainerType m_conditions;
};

using transitionContainerType = ReciclingVector<Transition>;
using transitionConstRefType = transitionContainerType::ConstRef;
using transitionRefType = transitionContainerType::Ref;

}
