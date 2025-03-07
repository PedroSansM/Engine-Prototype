#pragma once

#include "DommusCore.h"



namespace Game
{
	class KnockoutComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::KnockoutComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class KnockoutComponent : public DCore::ScriptComponent
{
public:
	KnockoutComponent(const DCore::ConstructorArgs<KnockoutComponent>&)
	{}
	~KnockoutComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override;
	virtual void OnAnimationEvent(size_t metachannelId) override;
public:
	void DisplayKnockoutMessage();
private:
	// Runtime
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	size_t m_displayAnimationParameter;
};

class KnockoutComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~KnockoutComponentScriptComponentFormGenerator() = default;
private:
	KnockoutComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"KnockoutComponent",
			DCore::ComponentId::GetId<KnockoutComponent>(),
			sizeof(KnockoutComponent),
			sizeof(DCore::ConstructorArgs<KnockoutComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) KnockoutComponent(*static_cast<const DCore::ConstructorArgs<KnockoutComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<KnockoutComponent*>(componentAddress)->~KnockoutComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<KnockoutComponent> m_defaultArgs;
private:
	static KnockoutComponentScriptComponentFormGenerator s_generator;
};

}
	