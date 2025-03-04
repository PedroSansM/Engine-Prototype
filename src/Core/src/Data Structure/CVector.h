#pragma once

#include "DCoreAssert.h"

#include <cstddef>
#include <cstring>



namespace DCore
{

class CVector
{
public:
	CVector(size_t chunkSize, size_t initialCapacity)
		:
		m_data(nullptr),
		m_chunkSize(chunkSize),
		m_occupation(0),
		m_capacity(initialCapacity)
	{
		DASSERT_E(m_chunkSize > 0 && m_capacity > 0);
		m_data = new char[m_capacity * m_chunkSize];
	}
		
	CVector(const CVector& other)
		:
		m_data(nullptr),
		m_chunkSize(other.m_chunkSize),
		m_occupation(other.m_occupation),
		m_capacity(other.m_capacity)
	{
		m_data = new char[m_capacity * m_chunkSize];
		if (m_occupation > 0)
		{
			std::memcpy(m_data, other.m_data, m_occupation * sizeof(m_chunkSize));
		}
	}

	CVector(CVector&& other) noexcept
		:
		m_data(other.m_data),
		m_chunkSize(other.m_chunkSize),
		m_occupation(other.m_occupation),
		m_capacity(other.m_capacity)
	{
		other.m_data = nullptr;
		other.m_chunkSize = 0;
		other.m_occupation = 0;
		other.m_capacity = 0;
	}

	~CVector()
	{
		delete[] m_data;
	}
public:
	void PushBack(void* data)
	{
		Resize();
		std::memcpy(m_data + m_occupation++ * m_chunkSize, data, m_chunkSize);
	}

	void* ReserveAtEnd()
	{
		Resize();
		return m_data + m_occupation++ * m_chunkSize;
	}

	void EraseAtIndex(size_t index)
	{
		DASSERT_E(index < m_occupation);
		char* removeBegin(m_data + index * m_chunkSize);
		char* removeEnd(removeBegin + m_chunkSize);
		if (m_occupation == 1)
		{
			m_occupation--;
			return;
		}
		if (removeEnd == m_data + m_occupation * m_chunkSize)
		{
			m_occupation--;
			return;
		}
		std::memmove(removeBegin, removeEnd, (m_occupation - index - 1) * m_chunkSize);
		m_occupation--;
	}

	size_t Size() const
	{
		return m_occupation;
	}

	size_t GetChunkSize() const
	{
		return m_chunkSize;
	}

	void* operator[](size_t index)
	{
		return m_data + index * m_chunkSize;
	}

	void* operator[](size_t index) const
	{
		return m_data + index * m_chunkSize;
	}

	CVector& operator=(CVector&& other) noexcept
	{
		delete[] m_data;
		m_data = other.m_data;
		m_chunkSize = other.m_chunkSize;
		m_occupation = other.m_occupation;
		m_capacity = other.m_capacity;
		other.m_data = nullptr;
		other.m_chunkSize = 0;
		other.m_occupation = 0;
		other.m_capacity = 0;
		return *this;
	}
private:
	char* m_data;
	size_t m_chunkSize;
	size_t m_occupation;
	size_t m_capacity;
private:
	void Resize()
	{
		if (m_occupation < m_capacity)
		{
			return;
		}
		m_capacity *= 2;
		char* newData(new char[m_capacity * m_chunkSize]);
		if (m_occupation > 0)
		{
			std::memcpy(newData, m_data, m_occupation * m_chunkSize);
		}
		delete[] m_data;
		m_data = newData;
	}	
};

}
