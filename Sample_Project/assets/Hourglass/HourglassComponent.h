#pragma once

#include "DommusCore.h"



namespace Game
{
	class HourglassComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::HourglassComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class HourglassComponent : public DCore::ScriptComponent
{
public:
	HourglassComponent(const DCore::ConstructorArgs<HourglassComponent>&) 
	{}
	~HourglassComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Awake() override;
public:
	void DisplayLoadingScreen();
	void HideLoadingScreen();
private:
	DCore::ComponentRef<DCore::AnimationStateMachineComponent> m_asmComponent;
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	size_t m_animateAnimationParameter;
	DCore::ComponentRef<DCore::SpriteComponent> m_loadingScreenSpriteComponent;
};

class HourglassComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~HourglassComponentScriptComponentFormGenerator() = default;
private:
	HourglassComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"HourglassComponent",
			DCore::ComponentId::GetId<HourglassComponent>(),
			sizeof(HourglassComponent),
			sizeof(DCore::ConstructorArgs<HourglassComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) HourglassComponent(*static_cast<const DCore::ConstructorArgs<HourglassComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<HourglassComponent*>(componentAddress)->~HourglassComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<HourglassComponent> m_defaultArgs;
private:
	static HourglassComponentScriptComponentFormGenerator s_generator;
};

}
	