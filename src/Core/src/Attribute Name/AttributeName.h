#pragma once

#include "DCoreAssert.h"

#include <string>
#include <vector>



namespace DCore
{

class AttributeName
{
public:
	using stringType = std::string;
	using nameContainerType = std::vector<stringType>;
public:
	AttributeName(const stringType& name);
	AttributeName(const AttributeName&);
	AttributeName(AttributeName&&);
	~AttributeName() = default;
public:
	AttributeName& operator=(AttributeName&&);
public:
	const stringType& GetName() const
	{
		DASSERT_E(!m_names.empty());
		return m_names[0];
	}

	const stringType& GetComponentAtIndex(size_t index) const
	{
		DASSERT_E(index + 1 < m_names.size());
		return m_names[index + 1];
	}
	
	size_t GetNumberOfComponents() const
	{
		return m_names.size() - 1;
	}
private:
	nameContainerType m_names;
};

}
