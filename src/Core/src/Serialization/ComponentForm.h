#pragma once

#include "ECSTypes.h"
#include "SerializationTypes.h"
#include "AttributeName.h"
#include "UUID.h"

#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>



namespace DCore
{

enum class AttributeType
{
	Integer,
	UInteger,
	Float,
	Size,
	Logic,
	Vector2,
	UIVector2,
	Vector3,
	String,
	UUID,
	EntityReference,
	SpriteMaterial,
	TaggedList,
	Color,
	Animation,
	AnimationStateMachine,
	PhysicsBodyType,
	PhysicsMaterial,
	PhysicsLayer,
	PhysicsLayers,
	SoundEventInstance,
};

enum class AttributeKeyframeType
{
	Integer,
	Float,
	NotDefined
};

class SerializedAttribute
{
public:
	SerializedAttribute(AttributeName&&, AttributeType, AttributeIdType);
	SerializedAttribute(const SerializedAttribute&);
	SerializedAttribute(SerializedAttribute&&) noexcept;
	~SerializedAttribute() = default;
public:
	const AttributeName& GetAttributeName() const
	{
		return m_attributeName;
	}

	AttributeType GetAttributeType() const
	{
		return m_attributeType;
	}
	
	AttributeIdType GetAttributeId() const
	{
		return m_attributeId;
	}

	size_t GetNumberOfAttributeComponents() const
	{
		switch (m_attributeType)
		{
		case AttributeType::Integer:
			return 1;
		case AttributeType::UInteger:
			return 1;
		case AttributeType::Float:
			return 1;
		case AttributeType::Size:
			return 1;
		case AttributeType::Logic:
			return 1;
		case AttributeType::Vector2:
			return 2;
		case AttributeType::UIVector2:
			return 2;
		case AttributeType::Vector3:
			return 3;
		case AttributeType::String:
			return 1;
		case AttributeType::UUID:
			return 1;
		case AttributeType::EntityReference:
			return 1;
		case AttributeType::SpriteMaterial:
			return 1;
		case AttributeType::TaggedList:
			return 1;
		case AttributeType::Color:
			return 4;
		case AttributeType::Animation:
			return 1;
		case AttributeType::AnimationStateMachine:
			return 1;
		case AttributeType::PhysicsMaterial:
			return 1;
		case AttributeType::PhysicsBodyType:
			return 1;
		case AttributeType::PhysicsLayer:
			return 1;
		case AttributeType::PhysicsLayers:
			return 1;
		case AttributeType::SoundEventInstance:
			return 1;
		}
		DASSERT_E(false);
		return 0;
	}

	AttributeKeyframeType GetKeyframeType() const
	{
		switch (m_attributeType)
		{
		case AttributeType::Integer:
			return AttributeKeyframeType::Integer;
		case AttributeType::UInteger:
			return AttributeKeyframeType::Integer;
		case AttributeType::Float:
			return AttributeKeyframeType::Float;
		case AttributeType::Size:
			return AttributeKeyframeType::Integer;
		case AttributeType::Logic:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::Vector2:
			return AttributeKeyframeType::Float;
		case AttributeType::UIVector2:
			return AttributeKeyframeType::Integer;
		case AttributeType::Vector3:
			return AttributeKeyframeType::Float;
		case AttributeType::String:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::UUID:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::EntityReference:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::SpriteMaterial:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::TaggedList:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::Color:
			return AttributeKeyframeType::Float;
		case AttributeType::Animation:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::AnimationStateMachine:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::PhysicsMaterial:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::PhysicsBodyType:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::PhysicsLayer:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::PhysicsLayers:
			return AttributeKeyframeType::NotDefined;
		case AttributeType::SoundEventInstance:
			return AttributeKeyframeType::NotDefined;
		}
		DASSERT_E(false);
		return AttributeKeyframeType::NotDefined;
	}

	// Only for non reference types!!
	size_t GetAttributeSizeBytes() const
	{
		switch (m_attributeType)
		{
		case AttributeType::Integer:
			return sizeof(DInt);
		case AttributeType::UInteger:
			return sizeof(DUInt);
		case AttributeType::Float:
			return sizeof(DFloat);
		case AttributeType::Size:
			return sizeof(DSize);
		case AttributeType::Logic:
			return sizeof(DLogic);
		case AttributeType::Vector2:
			return sizeof(DVec2);
		case AttributeType::UIVector2:
			return sizeof(DVec2);
		case AttributeType::Vector3:
			return sizeof(DVec3);
		case AttributeType::String:
			return sizeof(DString);
		case AttributeType::UUID:
			return sizeof(UUIDType);
		case AttributeType::TaggedList:
			// TODO
			return 1;
		case AttributeType::Color:
			return sizeof(DVec4);
		default:
			break;
		}
		DASSERT_E(false);
		return 0;
	}
	
	// Only for non reference types!!
	size_t GetAttributeComponentSizeBytes() const
	{
		return GetAttributeSizeBytes() / GetNumberOfAttributeComponents();
	}
public:
	SerializedAttribute& operator=(SerializedAttribute&&) noexcept;
private:
	AttributeName m_attributeName;
	AttributeType m_attributeType;
	AttributeIdType m_attributeId;
};

struct ComponentForm
{
	using serializedAttributeContainerType = std::vector<SerializedAttribute>;
	using constructorFunctionType = std::function<void(void*, const void*)>;
	using destructorFunctionType = std::function<void(void*)>;

	ComponentForm(
		ComponentIdType, 
		std::string&&, 
		bool, 
		size_t, 
		size_t, 
		serializedAttributeContainerType&&, 
		constructorFunctionType&&, 
		destructorFunctionType&&, 
		const void*);

	ComponentForm(ComponentForm&&) noexcept;

	ComponentIdType Id;
	std::string Name;
	bool IsScriptComponent;
	size_t TotalSize;
	size_t SerializedSize;
	serializedAttributeContainerType SerializedAttributes;
	constructorFunctionType PlacementNewConstructor;
	destructorFunctionType Destructor;
	const void* DefaultArgs;

	bool TryGetNumberOfAttributeComponents(AttributeIdType, size_t& out) const;

	ComponentForm& operator=(ComponentForm&& other) noexcept
	{
		Id = other.Id;
		Name = std::move(other.Name);
		IsScriptComponent = other.IsScriptComponent;
		TotalSize = other.TotalSize;
		SerializedSize = other.SerializedSize;
		SerializedAttributes = std::move(other.SerializedAttributes);
		PlacementNewConstructor = std::move(other.PlacementNewConstructor);
		Destructor = std::move(other.Destructor);
		DefaultArgs = other.DefaultArgs;
		other.DefaultArgs = nullptr;
		return *this;
	}

	bool operator==(const ComponentForm& other) const
	{
		return Id == other.Id;
	}

	bool operator!=(const ComponentForm& other) const
	{
		return Id != other.Id;
	}
};

class ComponentForms
{
public:
	using componentFormContainerType = std::vector<ComponentForm>;
	using componentFormWithNameContainerType = std::unordered_map<std::string, ComponentIdType>;
	using scriptComponentIdContainerType = std::vector<ComponentIdType>;
private:
	using ComponentFormComparator = struct ComponentFormComparator
	{
		bool operator()(const ComponentForm& a, const ComponentForm& b) const
		{
			return a.Id < b.Id;
		}
	};
public:
	ComponentForms(const ComponentForms&) = delete;
	ComponentForms(ComponentForms&&) = delete;
	~ComponentForms() = default;
public:
	void AddComponentForm(ComponentForm&& componentForm);
	const ComponentForm* GetComponentFormWithName(const std::string& componentName) const;
public:
	static ComponentForms& Get()
	{
		static ComponentForms componentForms;
		return componentForms;
	}
public:
	bool ExistsComponentWithId(ComponentIdType componentId) const
	{
		if (componentId == 0)
		{
			return false;
		}
		return componentId < m_componentForms.size();
	}

	bool ExistsComponentWithAttributeId(ComponentIdType componentId, AttributeIdType attributeId) const
	{
		return ExistsComponentWithId(componentId) && attributeId < m_componentForms[componentId - 1].SerializedAttributes.size();
	}

	const std::vector<ComponentForm>& GetComponentForms() const
	{
		return m_componentForms;
	}

	const scriptComponentIdContainerType& GetScriptComponentIds() const
	{
		return m_scriptComponentIds;
	}
public:
	const ComponentForm& operator[](ComponentIdType componentId)
	{
		DASSERT_E(componentId != 0);
		return m_componentForms[componentId - 1];
	}
private:
	ComponentForms() = default;
private:
	componentFormContainerType m_componentForms;
	componentFormWithNameContainerType m_componentFormWithName;
	scriptComponentIdContainerType m_scriptComponentIds;
};

class ComponentFormGenerator
{
public:
	~ComponentFormGenerator() = default;
protected:
	ComponentFormGenerator(ComponentForm&& componentForm)
	{
		ComponentForms::Get().AddComponentForm(std::move(componentForm));
	}
};

class ScriptComponentFormGenerator : private ComponentFormGenerator
{
public:
	virtual ~ScriptComponentFormGenerator() = default;
protected:
	ScriptComponentFormGenerator(
		std::string&& name, 
		ComponentIdType componentId, 
		size_t totalSize, 
		size_t serializedSize,
		std::vector<SerializedAttribute>&& serializedAttributes,
		std::function<void(void*, const void*)>&& placementNewConstructor,
		ComponentForm::destructorFunctionType&& destructor,
		const void* defaultArgs)
		:
		ComponentFormGenerator(
			{
				componentId,
				std::move(name),
				true,
				totalSize,
				serializedSize,
				std::move(serializedAttributes),
				std::move(placementNewConstructor),
				std::move(destructor),
				defaultArgs})
	{}
};

}
