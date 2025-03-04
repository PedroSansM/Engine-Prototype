#include "Condition.h"



namespace DCore
{

Condition::Condition(ParameterType parameterType,
	size_t parameterIndex,
	integerParameterContainerType& integerParameters, 
	floatParameterContainerType& floatParameters, 
	logicParameterContainerType& logicParameters, 
	triggerParameterContainerType& triggerParameters)
	:
	m_parameterType(parameterType),
	m_parameterIndex(parameterIndex),
	m_integerValue(0),
	m_floatValue(0.0f),
	m_logicValue{false},
	m_numericCondition(NumericConditionType::Bigger),
	m_integerParameters(integerParameters),
	m_floatParameters(floatParameters),
	m_logicParameters(logicParameters),
	m_triggerParameters(triggerParameters)
{}

Condition::Condition(
	const Condition& other,
	integerParameterContainerType& integerParameters, 
	floatParameterContainerType& floatParameters, 
	logicParameterContainerType& logicParameters, 
	triggerParameterContainerType& triggerParameters)
	:
	m_parameterType(other.m_parameterType),
	m_parameterIndex(other.m_parameterIndex),
	m_integerValue(other.m_integerValue),
	m_floatValue(other.m_floatValue),
	m_logicValue(other.m_logicValue),
	m_numericCondition(other.m_numericCondition),
	m_integerParameters(integerParameters),
	m_floatParameters(floatParameters),
	m_logicParameters(logicParameters),
	m_triggerParameters(triggerParameters)
{}

Condition::Condition(Condition&& other) noexcept
	:
	m_parameterType(other.m_parameterType),
	m_parameterIndex(other.m_parameterIndex),
	m_integerValue(other.m_integerValue),
	m_floatValue(other.m_floatValue),
	m_logicValue(other.m_logicValue),
	m_numericCondition(other.m_numericCondition),
	m_integerParameters(other.m_integerParameters),
	m_floatParameters(other.m_floatParameters),
	m_logicParameters(other.m_logicParameters),
	m_triggerParameters(other.m_triggerParameters)
{}

Condition::stringType Condition::GetNumericConditionTypeName(NumericConditionType numericCondition)
{
	stringType toReturn;
	switch (numericCondition)
	{
	case NumericConditionType::Bigger:
		toReturn = "Bigger";
		break;
	case NumericConditionType::Equals:
		toReturn = "Equals";
		break;
	case NumericConditionType::Smaller:
		toReturn = "Smaller";
		break;
	}
	return toReturn;
}

NumericConditionType Condition::GetNumericCondition(const stringType& name)
{
	if (name == "Bigger")
	{
		return NumericConditionType::Bigger;
	}
	if (name == "Equals")
	{
		return NumericConditionType::Equals;
	}
	if (name == "Smaller")
	{
		return NumericConditionType::Smaller;
	}
	DASSERT_E(false);
	return NumericConditionType::Smaller;
}

bool Condition::IsTrue()
{
	switch (m_parameterType)
	{
	case ParameterType::Integer:
	{
		integerParameterConstRefType parameter(GetParameter<ParameterType::Integer>());
		switch (m_numericCondition)
		{
		case NumericConditionType::Smaller:
			return parameter->GetValue() < m_integerValue;
		case NumericConditionType::Equals:
			return parameter->GetValue() == m_integerValue;
		case NumericConditionType::Bigger:
			return parameter->GetValue() > m_integerValue;
		default:
			DASSERT_E(false);
			break;
		}
		return false;
	}
	case ParameterType::Float:
	{
		floatParameterConstRefType parameter(GetParameter<ParameterType::Float>());
		switch (m_numericCondition)
		{
		case NumericConditionType::Smaller:
			return parameter->GetValue() < m_floatValue;
		case NumericConditionType::Equals:
			return parameter->GetValue() == m_floatValue;
		case NumericConditionType::Bigger:
			return parameter->GetValue() > m_floatValue;
		default:
			DASSERT_E(false);
			break;
		}
		return false;
	}
	case ParameterType::Logic:
	{
		logicParameterConstRefType parameter(GetParameter<ParameterType::Logic>());
		return parameter->GetValue().Value == m_logicValue.Value;
	}
	case ParameterType::Trigger:
	{
		triggerParameterRefType parameter(GetParameter<ParameterType::Trigger>());
		return parameter->GetValue().Value;
	}
	}
	return false;
}

}
