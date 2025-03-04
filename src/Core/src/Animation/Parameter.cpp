#include "Parameter.h"
#include "DCoreAssert.h"



namespace DCore::ParameterUtils
{

stringType GetParameterTypeName(ParameterType parameterType)
{
	
	switch (parameterType)
	{
	case ParameterType::Integer:
		return "Integer";
	case ParameterType::Float:
		return "Float";
	case ParameterType::Logic:
		return "Logic";
	case ParameterType::Trigger:
		return "Trigger";
	}
	DASSERT_E(false);
	return "";
}

ParameterType GetParameterType(const stringType& parameterName)
{
	if (parameterName == "Integer")
	{
		return ParameterType::Integer;
	}
	if (parameterName == "Float")
	{
		return ParameterType::Float;
	}
	if (parameterName == "Logic")
	{
		return ParameterType::Logic;
	}
	if (parameterName == "Trigger")
	{
		return ParameterType::Trigger;
	}
	DASSERT_E(false);
	return ParameterType::Integer;
}

}
