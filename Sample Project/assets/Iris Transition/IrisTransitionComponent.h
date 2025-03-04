#pragma once

#include "DommusCore.h"



namespace Game
{
	class IrisTransitionComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::IrisTransitionComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class IrisTransitionComponent : public DCore::ScriptComponent
{
public:
	IrisTransitionComponent(const DCore::ConstructorArgs<IrisTransitionComponent>&)
	{}
	~IrisTransitionComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override;
	virtual void OnMetachannelEvent(size_t eventId) override;
private:
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
};

class IrisTransitionComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~IrisTransitionComponentScriptComponentFormGenerator() = default;
private:
	IrisTransitionComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"IrisTransitionComponent",
			DCore::ComponentId::GetId<IrisTransitionComponent>(),
			sizeof(IrisTransitionComponent),
			sizeof(DCore::ConstructorArgs<IrisTransitionComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) IrisTransitionComponent(*static_cast<const DCore::ConstructorArgs<IrisTransitionComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<IrisTransitionComponent*>(componentAddress)->~IrisTransitionComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<IrisTransitionComponent> m_defaultArgs;
private:
	static IrisTransitionComponentScriptComponentFormGenerator s_generator;
};

}
	