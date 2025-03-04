#pragma once

#include "DCoreAssert.h"

#include <algorithm>
#include <cstddef>
#include <functional>



namespace DCore
{

template <class ValueType, size_t MaxSize>
class Array
{
public:
	using valueType = ValueType;
public:
	static constexpr size_t maxSize{MaxSize};
public:
	Array()
		:
		m_size(0)
	{}	

	Array(std::initializer_list<valueType> l)
		:
		m_size(l.size())
	{
		std::memcpy(m_container, l.begin(), m_size * sizeof(valueType));
	}

	Array(const Array& other)
		:
		m_size(other.m_size)
	{
		for (size_t i(0); i < m_size; i++)
		{
			new (&m_container[i]) valueType(other.m_container[i]);
		}
	}

	Array(Array&& other) noexcept
		:
		m_size(other.m_size)
	{
		for (size_t i(0); i < m_size; i++)
		{
			new (&m_container[i]) valueType(std::move(other.m_container[i]));
		}
		other.m_size = 0;
	}

	~Array() = default;
public:
	void PushBack(const ValueType& value)
	{
		DASSERT_E(m_size + 1 <= maxSize);
		new (&m_container[m_size++]) valueType(value);
	}

	void PushBack(ValueType&& value)
	{
		DASSERT_E(m_size + 1 <= maxSize);
		new (&m_container[m_size++]) valueType(std::move(value));
	}

	void EraseAt(size_t index)
	{
		if (index >= m_size)
		{
			return;
		}
		m_container[index].~valueType();
		for (size_t i(index); i < m_size - 1; i++)
		{
			m_container[index] = m_container[index + 1];
		}
		m_size--;
	}

	size_t Size() const
	{
		return m_size;
	}

	void Clear()
	{
		for (size_t i(0); i < m_size; i++)
		{
			m_container[i].~valueType();
		}
		m_size = 0;
	}

	valueType* begin()
	{
		return m_container;
	}

	valueType* end()
	{
		return m_container + m_size;
	}

	const valueType* begin() const
	{
		return m_container;
	}

	const valueType* end() const
	{
		return m_container + m_size;
	}
public:
	ValueType& operator[](size_t index)
	{
		DASSERT_E(index < maxSize);
		return m_container[index];
	}

	const ValueType& operator[](size_t index) const
	{
		DASSERT_E(index < maxSize);
		return m_container[index];
	}
private:
	valueType m_container[maxSize];
	size_t m_size;
};

}
