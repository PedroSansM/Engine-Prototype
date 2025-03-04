#pragma once

#include <cstddef>
#include <cstring>
#include <functional>



namespace DCore
{

class Vector
{
public:
	Vector(size_t chunkSize, size_t initialCapacity)
		:
		m_data(new char[initialCapacity * chunkSize]),
		m_chunkSize(chunkSize),
		m_occupation(0),
		m_capacity(initialCapacity)
	{
		std::memset(m_data, 0, m_capacity * m_chunkSize);
	}
	Vector(const Vector& other)
		:
		m_data(new char[other.m_capacity * other.m_chunkSize]),
		m_chunkSize(other.m_chunkSize),
		m_occupation(other.m_occupation),
		m_capacity(other.m_capacity)
	{
		std::memcpy(m_data, other.m_data, m_capacity * m_chunkSize);
	}
	Vector(Vector&& other) noexcept
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
	~Vector()
	{
		delete[] m_data;
		m_data = nullptr;
	}
public:
	class Iterator
	{
	public:
		Iterator(char* data, size_t chunkSize)
			:
			m_data(data),
			m_chunkSize(chunkSize)
		{}
		Iterator(const Iterator& other)
			:
			m_data(other.m_data),
			m_chunkSize(other.m_chunkSize)
		{}
		~Iterator() = default;
	public:
		template <class Type>
		Type& Get()
		{
			return *(Type*)m_data;
		}
	public:
		bool operator==(const Iterator& other) const
		{
			return m_data == other.m_data;
		}

		bool operator!=(const Iterator& other) const
		{
			return m_data != other.m_data;
		}

		void operator++(int)
		{
			m_data += m_chunkSize;
		}

		void operator=(const Iterator& other)
		{
			m_data = other.m_data;
			m_chunkSize = other.m_chunkSize;
		}
	private:
		char* m_data;
		size_t m_chunkSize;
	};
public:
	void PushBack()
	{
		MaybeResize();
		m_occupation++;
	}

	void PushBack(void* chunk)
	{
		MaybeResize();
		char* insertOffset(m_data + m_occupation * m_chunkSize);
		std::memcpy(insertOffset, chunk, m_chunkSize);
		std::memset(chunk, 0, m_chunkSize);
		m_occupation++;
	}
	
	void PushBack(const std::function<void(void*, const void*)>& constructor, const void* args)
	{
		MaybeResize();
		char* insertAddress(m_data + m_occupation * m_chunkSize);
		constructor(insertAddress, args);
		m_occupation++;
	}

	void Erase(size_t index)
	{
		if (index >= m_occupation)
		{
			return;
		}
		char* chunkToRemove(m_data + index * m_chunkSize);
		char* afterChunk(chunkToRemove + m_chunkSize);
		if (afterChunk != m_data + m_occupation * m_chunkSize) // Is not last element?
		{
			size_t moveSize(m_data + m_occupation * m_chunkSize - afterChunk);
			char* temp(new char[moveSize]);
			std::memcpy(temp, afterChunk, moveSize);
			std::memset(chunkToRemove, 0, moveSize + m_chunkSize);
			std::memcpy(chunkToRemove, temp, moveSize);
			delete[] temp;
		}
		m_occupation--;
	}
	
	size_t Size() const
	{
		return m_occupation;
	}
	
	Iterator begin()
	{
		return Iterator(m_data, m_chunkSize);
	}

	Iterator end()
	{
		return Iterator(m_data + m_occupation * m_chunkSize, m_chunkSize);
	}
		
	void* Get(size_t index)
	{
		if (index >= m_occupation)
		{
			return nullptr;
		}
		return (void*)(m_data + index * m_chunkSize);
	}

	const void* Get(size_t index) const
	{
		return (const void*)(m_data + index * m_chunkSize);
	}
	
	size_t GetChunkSize() const
	{
		return m_chunkSize;
	}

	void Fit()
	{
		if (m_occupation == 0)
		{
			delete[] m_data;
			m_data = new char[m_chunkSize];
			m_capacity = 1;
			return;
		}
		size_t fitSize(m_occupation * m_chunkSize);
		char* temp(new char[fitSize]);
		std::memcpy(temp, m_data, fitSize);
		delete[] m_data;
		m_data = nullptr;
		m_data = new char[fitSize];
		std::memcpy(m_data, temp, fitSize);
		m_capacity = m_occupation;
		delete[] temp;
	}
public:
	template <class Type, class ConstructionArgs>
	void PushBack(ConstructionArgs args)
	{
		MaybeResize();
		new (m_data + m_occupation * m_chunkSize) Type(args);
		m_occupation++;
	}

	template <class Type>
	void PushBack(Type&& data)
	{
		MaybeResize();
		new (m_data + m_occupation * m_chunkSize) Type(std::move(data));
		m_occupation++;
	}

	template <class Type>
	Type& Get(size_t index)
	{
		return *(Type*)(m_data + index * m_chunkSize);
	}

	template <class Type>
	const Type& Get(size_t index) const
	{
		return *(const Type*)(m_data + index * m_chunkSize);
	}

	template <class Type>
	bool TryGet(size_t index, Type** outData)
	{
		if (index >= m_occupation)
		{
			return false;
		}
		if (outData != nullptr)
		{
			*outData = (Type*)(m_data + index * m_chunkSize);
		}
		return true;
	}
	
	template <class Type>
	Type* Data()
	{
		return (Type*)m_data;
	}
	
	template <class Type>
	void CallDestructors()
	{
		if (sizeof(Type) != m_chunkSize)
		{
			return;
		}
		for (size_t i(0); i < m_occupation; i++)
		{
			((Type*)m_data)[i].~Type();
		}
	}
public:
	Vector& operator=(const Vector& other)
	{
		delete[] m_data;
		m_data = nullptr;
		size_t otherTotalSize(other.m_capacity * other.m_chunkSize);
		m_data = new char[otherTotalSize];
		std::memcpy(m_data, other.m_data, otherTotalSize);
		m_chunkSize = other.m_chunkSize;
		m_occupation = other.m_occupation;
		m_capacity = other.m_capacity;
		return *this;
	}
private:
	char* m_data;
	size_t m_chunkSize;
	size_t m_occupation;
	size_t m_capacity;
private:
	void MaybeResize()
	{
		if (m_occupation + 1 < 0.75f * m_capacity)
		{
			return;
		}
		char* temp(new char[m_occupation * m_chunkSize]);			// In heap to not (potentially) blow up the stack.
		std::memcpy(temp, m_data, m_occupation * m_chunkSize);
		delete[] m_data;
		m_data = nullptr;
		m_capacity *= 2;
		m_data = new char[m_capacity * m_chunkSize];
		std::memset(m_data, 0, m_capacity * m_chunkSize);
		std::memcpy(m_data, temp, m_occupation * m_chunkSize);
		delete[] temp;
	}
};

}
