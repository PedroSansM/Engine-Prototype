#pragma once

#include "ReciclingVector.h"

#include <string>
#include <type_traits>



namespace DCore
{

struct LogicParameter
{
	bool Value;
};

struct TriggerParameter
{
	bool Value;
};

enum class ParameterType
{
	Integer,
	Float, 
	Logic,
	Trigger
};

namespace ParameterUtils
{
	using stringType = std::string;

	stringType GetParameterTypeName(ParameterType);
	ParameterType GetParameterType(const stringType&);
};

template <class ValueType>
class Parameter
{
	static_assert(std::is_same_v<int, ValueType> || std::is_same_v<float, ValueType> || std::is_same_v<LogicParameter, ValueType> || std::is_same_v<TriggerParameter, ValueType>, "Only int, float, logic and trigger parameter types are allowed.");
public:
	using stringType = std::string;
	using valueType = ValueType;
public:
	static constexpr ParameterType parameterType{std::is_same_v<int, ValueType> ? ParameterType::Integer : (std::is_same_v<float, ValueType> ? ParameterType::Float : ParameterType::Logic)};
public:
	Parameter(const stringType& name, valueType initialValue)
		:
		m_name(name),
		m_value(initialValue)
	{}
	Parameter(const Parameter& other)
		:
		m_name(other.m_name),
		m_value(other.m_value)
	{}
	Parameter(Parameter&& other) noexcept
		:
		m_name(std::move(other.m_name)),
		m_value(other.m_value)
	{}
	~Parameter() = default;	
public:
	const stringType& GetName() const
	{
		return m_name;
	}

	valueType GetValue() const
	{
		return m_value;
	}

	decltype(auto) GetLiteralValue() const
	{
		if constexpr (parameterType == ParameterType::Logic)
		{
			return m_value.Value;
		}
		else
		{
			return m_value;
		}
	}

	ParameterType GetType() const
	{
		return parameterType;
	}

	void SetName(const stringType& name)
	{
		m_name = name;
	}

	void SetValue(valueType value)
	{
		m_value = value;
	}
private:
	stringType m_name;
	valueType m_value;
};

template <>
class Parameter<TriggerParameter>
{
public:
	using stringType = std::string;
	using valueType = TriggerParameter;
public:
	static constexpr ParameterType parameterType{ParameterType::Trigger};
public:
	Parameter(const stringType& name, valueType initialValue)
		:
		m_name(name),
		m_value(initialValue)
	{}
	Parameter(const Parameter& other)
		:
		m_name(other.m_name),
		m_value(other.m_value)
	{}
	Parameter(Parameter&& other) noexcept
		:
		m_name(std::move(other.m_name)),
		m_value(other.m_value)
	{}
	~Parameter() = default;	
public:
	const stringType& GetName() const
	{
		return m_name;
	}

	valueType GetValue()
	{
		valueType value;
		value.Value = m_value.Value;
		m_value.Value = false;
		return value;
	}
	
	valueType GetValueWithoutReset() const
	{
		return m_value;
	}

	bool GetLiteralValue() const
	{
		return m_value.Value;
	}

	ParameterType GetType() const
	{
		return parameterType;
	}

	void SetName(const stringType& name)
	{
		m_name = name;
	}

	void SetValue(valueType value)
	{
		m_value = value;
	}
private:
	stringType m_name;
	valueType m_value;
};

template <class ValueType>
using parameterContainerType = ReciclingVector<Parameter<ValueType>>;

using integerParameterContainerType = parameterContainerType<int>;
using floatParameterContainerType = parameterContainerType<float>;
using logicParameterContainerType = parameterContainerType<LogicParameter>;
using triggerParameterContainerType = parameterContainerType<TriggerParameter>;

using integerParameterRefType = integerParameterContainerType::Ref;
using floatParameterRefType = floatParameterContainerType::Ref;
using logicParameterRefType = logicParameterContainerType::Ref;
using triggerParameterRefType = triggerParameterContainerType::Ref;
using integerParameterConstRefType = integerParameterContainerType::ConstRef;
using floatParameterConstRefType = floatParameterContainerType::ConstRef;
using logicParameterConstRefType = logicParameterContainerType::ConstRef;
using triggerParameterConstRefType = triggerParameterContainerType::ConstRef;

}
