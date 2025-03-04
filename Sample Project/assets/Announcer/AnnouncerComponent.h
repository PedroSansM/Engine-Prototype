#pragma once

#include "DommusCore.h"



namespace Game
{
	class AnnouncerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::AnnouncerComponent>
{
	ConstructorArgs()
	{}

	DCore::DSoundEventInstance ReadySound;
	DCore::DSoundEventInstance BeginSound;
	float DelayToStartReadySound;
	float DelayToStartBeginSound;
};
#pragma pack(pop)

namespace Game
{

class AnnouncerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_readySound{0};
	static constexpr size_t a_beginSound{1};
	static constexpr size_t a_delayToStartReadySound{2};
	static constexpr size_t a_delayToStartBeginSound{3};
public:
	AnnouncerComponent(const DCore::ConstructorArgs<AnnouncerComponent>&);
	~AnnouncerComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
private:
	// Editor
	DCore::DSoundEventInstance m_readySound;
	DCore::DSoundEventInstance m_beginSound;
	float m_delayToStartReadySound;
	float m_delayToStartBeginSound;
	// Runtime
	float m_currentDelayToStartReadySound;
	float m_currentDelayToStartBeginSound;
};

class AnnouncerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~AnnouncerComponentScriptComponentFormGenerator() = default;
private:
	AnnouncerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"AnnouncerComponent",
			DCore::ComponentId::GetId<AnnouncerComponent>(),
			sizeof(AnnouncerComponent),
			sizeof(DCore::ConstructorArgs<AnnouncerComponent>),
			{{DCore::AttributeName("Ready Sound"), DCore::AttributeType::SoundEventInstance, AnnouncerComponent::a_readySound},
			{DCore::AttributeName("Begin Sound"), DCore::AttributeType::SoundEventInstance, AnnouncerComponent::a_beginSound},
			{DCore::AttributeName("Delay To Start Ready Sound"), DCore::AttributeType::Float, AnnouncerComponent::a_delayToStartReadySound},
			{DCore::AttributeName("Delay To Start Begin Sound"), DCore::AttributeType::Float, AnnouncerComponent::a_delayToStartBeginSound}},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) AnnouncerComponent(*static_cast<const DCore::ConstructorArgs<AnnouncerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<AnnouncerComponent*>(componentAddress)->~AnnouncerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<AnnouncerComponent> m_defaultArgs;
private:
	static AnnouncerComponentScriptComponentFormGenerator s_generator;
};

}
	