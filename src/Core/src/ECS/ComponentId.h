#pragma once

#include "ECSTypes.h"



namespace DCore
{

class ComponentId
{
public:
	ComponentId(const ComponentId&) = delete;
	ComponentId(ComponentId&&) = delete;
	~ComponentId() = default;
public:
	template <class ComponentType>
	static ComponentIdType GetId()
	{
		static ComponentIdType componentId(s_nextComponentId++);
		return componentId;
	}
private:
	ComponentId() = default;	
private:
	static ComponentIdType s_nextComponentId;
};

}
