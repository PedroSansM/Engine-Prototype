#include "Transition.h"
#include "Condition.h"

#include <cstddef>



namespace DCore
{

Transition::Transition(size_t fromStateIndex, size_t toStateId)
	:
	m_fromStateIndex(fromStateIndex),
	m_toStateIndex(toStateId)
{}

Transition::Transition(const Transition& other,
		integerParameterContainerType& integerParameters,
		floatParameterContainerType& floatParameters,
		logicParameterContainerType& logicParameters,
		triggerParameterContainerType& triggerParameters)
	:
	m_fromStateIndex(other.m_fromStateIndex),
	m_toStateIndex(other.m_toStateIndex)
{
	other.m_conditions.IterateConstRef
	(
		[&](conditionConstRefType condition) -> bool
		{
			m_conditions.PushBack(Condition(*condition.Data(), integerParameters, floatParameters, logicParameters, triggerParameters));
			return false;
		}
	);
}

Transition::Transition(Transition&& other) noexcept
	:
	m_fromStateIndex(other.m_fromStateIndex),
	m_toStateIndex(other.m_toStateIndex),
	m_conditions(std::move(other.m_conditions))
{}

conditionRefType Transition::CreateCondition(
		ParameterType conditionParameter, 
		size_t parameterIndex, 
		integerParameterContainerType& integerParameters, 
		floatParameterContainerType& floatParameters, 
		logicParameterContainerType& logicParameters, 
		triggerParameterContainerType& triggerParameters)
{
	return m_conditions.PushBack(conditionParameter, parameterIndex, integerParameters, floatParameters, logicParameters, triggerParameters);
}

bool Transition::ToExecute()
{
	bool toExecute(true);
	m_conditions.Iterate
	(
		[&](conditionRefType condition) -> bool
		{
			if (!condition->IsTrue())
			{
				toExecute = false;
				return true;
			}
			return false;
		}
	);
	return toExecute;
}

void Transition::DeleteCondition(size_t conditionIndex)
{
	m_conditions.RemoveElementAtIndex(conditionIndex);
}

Transition& Transition::operator=(Transition&& other) noexcept
{
	m_toStateIndex = other.m_toStateIndex;
	return *this;
}

}
