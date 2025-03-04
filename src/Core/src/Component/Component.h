#pragma once

#include "AssetManagerTypes.h"
#include "SerializationTypes.h"
#include "ReadWriteLockGuard.h"
#include "ComponentForm.h"



namespace DCore
{

class Component
{
public:
	virtual ~Component() = default;
public:
	virtual void* GetAttributePtr(AttributeIdType)
	{
		return nullptr;
	}

	virtual void OnAttributeChange(AttributeIdType, void* newValue, AttributeType typeHint)
	{}
protected:
	Component() = default;
};

}
