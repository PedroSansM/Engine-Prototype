#include "AttributeName.h"



namespace DCore
{

AttributeName::AttributeName(const std::string& name)
{
	std::string currentName;
	currentName.reserve(name.size());
	for (char c : name)
	{
		if (c == '#')
		{
			m_names.push_back(currentName);
			currentName.clear();
			continue;
		}
		currentName.push_back(c);
	}
	m_names.push_back(currentName);
}

AttributeName::AttributeName(const AttributeName& other)
	:
	m_names(other.m_names)
{}

AttributeName::AttributeName(AttributeName&& other)
	:
	m_names(std::move(other.m_names))
{}

AttributeName& AttributeName::operator=(AttributeName&& other)
{
	m_names = std::move(other.m_names);
	return *this;
}

}
