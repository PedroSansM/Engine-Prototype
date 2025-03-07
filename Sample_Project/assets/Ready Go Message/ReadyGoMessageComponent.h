#pragma once

#include "DommusCore.h"



namespace Game
{
	class ReadyGoMessageComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::ReadyGoMessageComponent>
{
	ConstructorArgs()
	{}
	
	float PresentationDelay;
};
#pragma pack(pop)

namespace Game
{

class ReadyGoMessageComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_presentationDelay{0};
public:
	ReadyGoMessageComponent(const DCore::ConstructorArgs<ReadyGoMessageComponent>&);
	~ReadyGoMessageComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
	virtual void OnAnimationEvent(size_t eventId) override;
private:
	// Editor
	float m_presentationDelay;
	// Runtime
	float m_currentPresentationDelay;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	size_t m_displayAnimationParameter;
};

class ReadyGoMessageComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~ReadyGoMessageComponentScriptComponentFormGenerator() = default;
private:
	ReadyGoMessageComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"ReadyGoMessageComponent",
			DCore::ComponentId::GetId<ReadyGoMessageComponent>(),
			sizeof(ReadyGoMessageComponent),
			sizeof(DCore::ConstructorArgs<ReadyGoMessageComponent>),
			{{DCore::AttributeName("Presentation Delay"), DCore::AttributeType::Float, ReadyGoMessageComponent::a_presentationDelay}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) ReadyGoMessageComponent(*static_cast<const DCore::ConstructorArgs<ReadyGoMessageComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<ReadyGoMessageComponent*>(componentAddress)->~ReadyGoMessageComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<ReadyGoMessageComponent> m_defaultArgs;
private:
	static ReadyGoMessageComponentScriptComponentFormGenerator s_generator;
};

}
	