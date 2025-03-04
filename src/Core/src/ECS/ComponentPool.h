#pragma once

#include "ECSTypes.h"
#include "CVector.h"

#include <cstddef>



namespace DCore
{

class ComponentPool
{
public:
	static constexpr size_t initialComponentsCapacity{5};
public:
	ComponentPool(size_t chunkSize)
		:
		m_components(chunkSize, initialComponentsCapacity)
	{}

	ComponentPool(const ComponentPool& other)
		:
		m_components(other.m_components)
	{}

	ComponentPool(ComponentPool&& other) noexcept
		:
		m_components(std::move(other.m_components))
	{}
public:
	void* AddComponent()
	{
		return m_components.ReserveAtEnd();
	}
	
	void AddComponent(void* component)
	{
		m_components.PushBack(component);
	}

	void RemoveComponent(size_t index)
	{
		m_components.EraseAtIndex(index);
	}

	void* GetComponentAtIndex(size_t index) const
	{
		return m_components[index];
	}

	void MoveComponentAtIndexTo(size_t index, ComponentPool& other)
	{
		void* component(m_components[index]);
		other.AddComponent(component);
		m_components.EraseAtIndex(index);
	}

	size_t GetChunkSize() const
	{
		return m_components.GetChunkSize();
	}
private:
	CVector m_components;
};

}
