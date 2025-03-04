#pragma once

#include "ComponentId.h"
#include "TemplateUtils.h"

#include <cstddef>



namespace DCore
{

template <class Component, class ...Components>
class ComponentTypesToComponentIds
{
public:
	static constexpr size_t numberOfComponents{TypeList<Component, Components...>::size};
public:
	ComponentTypesToComponentIds()
	{
		GetComponentId(0, TypeList<Component, Components...>{});
	}
public:
	ComponentIdType* Get()
	{
		return m_componentIds;
	}

	const ComponentIdType* Get() const
	{
		return m_componentIds;
	}
private:
	ComponentIdType m_componentIds[numberOfComponents];
private:
	void GetComponentId(size_t index, TypeList<>)
	{}
private:
	template <class ComponentT, class ...ComponentsT >
	void GetComponentId(size_t index, TypeList<ComponentT, ComponentsT...>) 
	{
		m_componentIds[index] = ComponentId::GetId<ComponentT>();
		GetComponentId(index + 1, TypeList<ComponentsT...>{});
	}
};

template <class Component, class ...Components>
class ComponentTypesToComponentSizes
{
public:
	static constexpr size_t numberOfComponents{TypeList<Component, Components...>::size};
public:
	ComponentTypesToComponentSizes()
	{
		GetComponentSize(0, TypeList<Component, Components...>{});
	}
public:
	size_t* Get()
	{
		return m_componentSizes;
	}
private:
	size_t m_componentSizes[numberOfComponents];
private:
	void GetComponentSize(size_t index, TypeList<>)
	{}
private:
	template <class ComponentT, class ...ComponentsT>
	void GetComponentSize(size_t index, TypeList<ComponentT, ComponentsT...>)
	{
		m_componentSizes[index] = sizeof(ComponentT);
		GetComponentSize(index + 1, TypeList<ComponentsT...>{});
	}
};

struct SortComponentIdsAndSizes
{
	void operator()(ComponentIdType* components, size_t* sizes, size_t numberOfComponents) const
	{
		bool switched(false);
		do
		{
			for (size_t i(0); i < numberOfComponents - 1; i++)
			{
				if (components[i] > components[i + 1])
				{
					switched = true;
					ComponentIdType componentIdTemp(components[i]);
					components[i] = components[i + 1];
					components[i + 1] = componentIdTemp;
					size_t sizeTemp(sizes[i]);
					sizes[i] = sizes[i + 1];
					sizes[i + 1]  = sizeTemp;
				}
			}
		} while(switched);
	}
};

}
