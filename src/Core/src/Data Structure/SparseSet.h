#pragma once

#include "DCoreAssert.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <type_traits>
#include <vector>



namespace DCore
{

template <class ValueType = size_t>
class SparseSet
{
	static_assert(std::is_unsigned<ValueType>::value, "Sparse sets can only have values of unsigned arithmetic types.");
public:
	using valueType = ValueType;
	using valueContainerType = std::vector<valueType>;
	using sparseContainerType = std::vector<size_t>;
public:
	SparseSet(valueType maxValue = 1)
		:
		m_maxValue(maxValue)
	{
		m_sparse.resize(maxValue + 1);
	}
	SparseSet(const SparseSet& other)
		:
		m_maxValue(other.m_maxValue),
		m_dense(other.m_dense),
		m_sparse(other.m_sparse)
	{}
	SparseSet(SparseSet&& other) noexcept
		:
		m_maxValue(other.m_maxValue),
		m_dense(std::move(other.m_dense)),
		m_sparse(std::move(other.m_sparse))
	{
		other.m_maxValue = 0;
	}
	~SparseSet() = default;
public:
	void Add(valueType value)
	{
		if (value > m_maxValue)
		{
			m_maxValue = value;
			m_sparse.resize(m_maxValue + 1);
		}
		m_sparse[value] = m_dense.size();
		m_dense.push_back(value);
	}

	void Remove(valueType value)
	{
		if (!Exists(value))
		{
			return;
		}
		if (m_dense.size() == 1)
		{
			m_dense.pop_back();
			return;
		}
		valueType denseBack(m_dense.back());
		m_dense[m_sparse[value]] = denseBack;
		m_sparse[denseBack] = m_sparse[value];
		m_dense.pop_back();
	}

	void KeepOrderRemove(valueType value)
	{
		if(!Exists(value))
		{
			return;
		}
		size_t removalIndex(m_sparse[value]);
		m_dense.erase(m_dense.begin() + removalIndex);
		size_t iterationIndex(0);
		for (ValueType denseValue : m_dense)
		{
			m_sparse[denseValue] = iterationIndex++;
		}
	}

	bool Exists(valueType value) const
	{
		if (value > m_maxValue || m_dense.empty())
		{
			return false;
		}
		size_t index(m_sparse[value]);
		return index < m_dense.size() && m_dense[index] == value;
	}

	void Clear()
	{
		m_dense.clear();
	}

	size_t Size() const
	{
		return m_dense.size();
	}

	bool TryGetIndexTo(valueType value, size_t* outIndex) const
	{
		if (!Exists(value))
		{
			return false;
		}
		*outIndex = m_sparse[value];
		return true;
	}
	
	size_t GetIndexTo(valueType value) const
	{
		DASSERT_E(Exists(value));
		return m_sparse[value];
	}

	valueContainerType GetDenseCopy() const
	{
		return m_dense;
	}
	
	valueContainerType& GetDenseRef()
	{
		return m_dense;
	}

	const valueContainerType& GetDenseRef() const
	{
		return m_dense;
	}

	const sparseContainerType& GetSparseRef() const
	{
		return m_sparse;
	}

 	typename valueContainerType::iterator begin()
	{
		return m_dense.begin();
	}

	typename valueContainerType::iterator end()
	{
		return m_dense.end();
	}

	typename valueContainerType::const_iterator begin() const
	{
		return m_dense.begin();
	}

	typename valueContainerType::const_iterator end() const
	{
		return m_dense.end();
	}
public:
	valueType& operator[](size_t denseIndex)
	{
		DASSERT_E(denseIndex < m_dense.size());
		m_dense[denseIndex];
	}

	const valueType& operator[](size_t denseIndex) const
	{
		DASSERT_E(denseIndex < m_dense.size());
		m_dense[denseIndex];
	}

	SparseSet& operator=(const SparseSet& other)
	{
		m_dense = other.m_dense;
		m_sparse = other.m_sparse;
		m_maxValue = other.m_maxValue;
		return *this;
	}
	
	SparseSet& operator=(SparseSet&& other) noexcept
	{
		m_dense = std::move(other.m_dense);
		m_sparse = std::move(other.m_sparse);
		m_maxValue = other.m_maxValue;
		other.m_maxValue = 0;
		return *this;
	}
private:
	valueContainerType m_dense;
	sparseContainerType m_sparse;
	valueType m_maxValue;
};

}
