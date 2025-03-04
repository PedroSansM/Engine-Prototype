#pragma once

#include "Parameter.h"

#include <type_traits>
#include <string>



namespace DCore
{

enum class NumericConditionType
{
	Smaller,
	Equals,
	Bigger
};

class Condition
{
public:
	using stringType = std::string;
public:
	Condition(ParameterType, 
			size_t parameterIndex,
			integerParameterContainerType&, 
			floatParameterContainerType&, 
			logicParameterContainerType&, 
			triggerParameterContainerType&);
	Condition(const Condition&,
			integerParameterContainerType&, 
			floatParameterContainerType&, 
			logicParameterContainerType&, 
			triggerParameterContainerType&);
	Condition(Condition&&) noexcept;
	~Condition() = default;
public:
	static stringType GetNumericConditionTypeName(NumericConditionType);
	static NumericConditionType GetNumericCondition(const stringType&);
public:
	bool IsTrue();
public:
	ParameterType GetParameterType() const
	{
		return m_parameterType;
	}

	size_t GetParameterIndex() const
	{
		return m_parameterIndex;
	}

	void SetParameterType(ParameterType parameterType)
	{
		m_parameterType = parameterType;
		m_integerValue = 0;
		m_floatValue = 0.0f;
		m_logicValue.Value = false;
	}

	void SetParameterIndex(size_t parameterIndex)
	{
		m_parameterIndex = parameterIndex;
	}
	
	void SetNumericCondition(NumericConditionType numericCondition)
	{
		m_numericCondition = numericCondition;
	}

	NumericConditionType GetNumericCondition() const
	{
		return m_numericCondition;
	}

	void SetIntegerValue(int value)
	{
		m_integerValue = value;
	}

	void SetFloatValue(float value)
	{
		m_floatValue = value;
	}

	void SetLogicValue(LogicParameter value)
	{
		m_logicValue = value;
	}
public:
	template <ParameterType ParameterT>
	decltype(auto) GetParameter() const
	{
		DASSERT_E(ParameterT == m_parameterType);	
		if constexpr (ParameterT == ParameterType::Integer)
		{
			return m_integerParameters.GetRefFromIndex(m_parameterIndex);
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			return m_floatParameters.GetRefFromIndex(m_parameterIndex);
		}
		else if constexpr (ParameterT == ParameterType::Logic)
		{
			return m_logicParameters.GetRefFromIndex(m_parameterIndex);
		}
		else
		{
			return m_triggerParameters.GetRefFromIndex(m_parameterIndex);
		}	
	}
	
	template <ParameterType ParameterT>
	decltype(auto) GetValue() const
	{
		static_assert(ParameterT != ParameterType::Trigger, "Trigger conditions don't have value.");
		DASSERT_E(ParameterT == m_parameterType);	
		if constexpr (ParameterT == ParameterType::Integer)
		{
			return m_integerValue;
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			return m_floatValue;
		}
		else 
		{
			return m_logicValue;
		}
	}
	
	template <ParameterType ParameterT, class ValueType>
	void SetValue(ValueType value)
	{
		static_assert(ParameterT != ParameterType::Trigger, "Trigger conditions don't have value.");
		static_assert((ParameterT == ParameterType::Integer && std::is_same_v<ValueType, int>) ||
						(ParameterT == ParameterType::Float && std::is_same_v<ValueType, float>) ||
						(ParameterT == ParameterType::Logic && std::is_same_v<ValueType, LogicParameter>),
						"Invalid combination o parameter and value.");
		DASSERT_E(ParameterT == m_parameterType);	
		if constexpr (ParameterT == ParameterType::Integer)
		{
			m_integerValue = value;
		}
		else if constexpr (ParameterT == ParameterType::Float)
		{
			m_floatValue = value;
		}
		else
		{
			m_logicValue = value;
		}
	}
private:
	ParameterType m_parameterType;
	size_t m_parameterIndex;
	int m_integerValue;
	float m_floatValue;
	LogicParameter m_logicValue;
	NumericConditionType m_numericCondition;
	integerParameterContainerType& m_integerParameters;
	floatParameterContainerType& m_floatParameters;
	logicParameterContainerType& m_logicParameters;
	triggerParameterContainerType& m_triggerParameters;
};

using conditionContainerType = ReciclingVector<Condition>;
using conditionRefType = conditionContainerType::Ref;
using conditionConstRefType = conditionContainerType::ConstRef;

}
