#include "ComponentForm.h"



namespace DCore
{

SerializedAttribute::SerializedAttribute(AttributeName&& attributeName, AttributeType attributeType, AttributeIdType attributeId)
	:
	m_attributeName(std::move(attributeName)),
	m_attributeType(attributeType),
	m_attributeId(attributeId)
{}

SerializedAttribute::SerializedAttribute(const SerializedAttribute& other)
	:
	m_attributeName(other.m_attributeName),
	m_attributeType(other.m_attributeType),
	m_attributeId(other.m_attributeId)
{}

SerializedAttribute::SerializedAttribute(SerializedAttribute&& other) noexcept
	:
	m_attributeName(std::move(other.m_attributeName)),
	m_attributeType(other.m_attributeType),
	m_attributeId(other.m_attributeId)
{}

SerializedAttribute& SerializedAttribute::operator=(SerializedAttribute&& other) noexcept
{
	m_attributeName = std::move(other.m_attributeName);
	m_attributeType = other.m_attributeType;
	m_attributeId = other.m_attributeId;
	return *this;
}
 
ComponentForm::ComponentForm(
	ComponentIdType id, 
	std::string&& name, 
	bool isScriptComponent, 
	size_t totalSize, 
	size_t serializedSize, 
	serializedAttributeContainerType&& serializedAttributes, 
	constructorFunctionType&& placementNewConstructor, 
	destructorFunctionType&& destructor, 
	const void* defaultArgs)
	:
	Id(id),
	Name(std::move(name)),
	IsScriptComponent(isScriptComponent),
	TotalSize(totalSize),
	SerializedSize(serializedSize),
	SerializedAttributes(std::move(serializedAttributes)),
	PlacementNewConstructor(std::move(placementNewConstructor)),
	Destructor(std::move(destructor)),
	DefaultArgs(defaultArgs)
{}

ComponentForm::ComponentForm(ComponentForm&& other) noexcept
	:
	Id(other.Id),
	Name(std::move(other.Name)),
	IsScriptComponent(other.IsScriptComponent),
	TotalSize(other.TotalSize),
	SerializedSize(other.SerializedSize),
	SerializedAttributes(std::move(other.SerializedAttributes)),
	PlacementNewConstructor(std::move(other.PlacementNewConstructor)),
	Destructor(std::move(other.Destructor)),
	DefaultArgs(other.DefaultArgs)
{
	other.DefaultArgs = nullptr;
}

bool ComponentForm::TryGetNumberOfAttributeComponents(DCore::AttributeIdType attributeId, size_t &out) const
{
	if (attributeId >= SerializedAttributes.size())
	{
		return false;
	}
	out = SerializedAttributes[attributeId].GetNumberOfAttributeComponents();
	return true;
}

void ComponentForms::AddComponentForm(ComponentForm&& componentForm)
{
	m_componentForms.push_back(std::move(componentForm));
	m_componentFormWithName.insert({m_componentForms.back().Name, m_componentForms.back().Id});
	if (m_componentForms.back().IsScriptComponent)
	{
		m_scriptComponentIds.push_back(m_componentForms.back().Id);
	}
	std::sort(m_componentForms.begin(), m_componentForms.end(), ComponentFormComparator());
}

const ComponentForm* ComponentForms::GetComponentFormWithName(const std::string& componentName) const
{
	auto iterator(m_componentFormWithName.find(componentName));
	if (iterator == m_componentFormWithName.end())
	{
		return nullptr;
	}
	return &m_componentForms[iterator->second - 1];
}

}
