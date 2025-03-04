#pragma once

#include "DommusCore.h"



namespace Game
{
	class YouDiedMessageComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::YouDiedMessageComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class YouDiedMessageComponent : public DCore::ScriptComponent
{
public:
	YouDiedMessageComponent(const DCore::ConstructorArgs<YouDiedMessageComponent>&)
	{}
	~YouDiedMessageComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override;
	virtual void OnMetachannelEvent(size_t metachannelId) override;
public:
	void Display();
private:
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_backgroundSpriteComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_pressRToRestartSpriteComponent;
	size_t m_displayAnimationParameter;
};

class YouDiedMessageComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~YouDiedMessageComponentScriptComponentFormGenerator() = default;
private:
	YouDiedMessageComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"YouDiedMessageComponent",
			DCore::ComponentId::GetId<YouDiedMessageComponent>(),
			sizeof(YouDiedMessageComponent),
			sizeof(DCore::ConstructorArgs<YouDiedMessageComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) YouDiedMessageComponent(*static_cast<const DCore::ConstructorArgs<YouDiedMessageComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<YouDiedMessageComponent*>(componentAddress)->~YouDiedMessageComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<YouDiedMessageComponent> m_defaultArgs;
private:
	static YouDiedMessageComponentScriptComponentFormGenerator s_generator;
};

}
	