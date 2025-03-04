#pragma once

#include "DommusCore.h"



namespace Game
{
	class MainMenuControllerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::MainMenuControllerComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class MainMenuControllerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t numberOfOptions{ 2 };
	static constexpr size_t numberOfBackgroundElements{ 2 };
	static constexpr size_t startOptionIndex{ 0 };
	static constexpr size_t controlsOptionIndex{ 1 };
public:
	MainMenuControllerComponent(const DCore::ConstructorArgs<MainMenuControllerComponent>&)
	{}
	~MainMenuControllerComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override
	{
		return nullptr;
	}

	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override
	{}
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
public:
	void Enable();
private:
	bool m_enabled;
	DCore::ComponentRef<DCore::SpriteComponent> m_optionSpriteComponents[numberOfOptions];
	DCore::Array<DCore::ComponentRef<DCore::SpriteComponent>, numberOfBackgroundElements> m_backgroundElementSpriteComponents;
	DCore::ComponentRef<DCore::Component> m_gameManagerComponent;
	size_t m_selectedOptionIndex;
private:
	void HandleOptionsHighlight();
};

class MainMenuControllerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~MainMenuControllerComponentScriptComponentFormGenerator() = default;
private:
	MainMenuControllerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"MainMenuControllerComponent",
			DCore::ComponentId::GetId<MainMenuControllerComponent>(),
			sizeof(MainMenuControllerComponent),
			sizeof(DCore::ConstructorArgs<MainMenuControllerComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) MainMenuControllerComponent(*static_cast<const DCore::ConstructorArgs<MainMenuControllerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<MainMenuControllerComponent*>(componentAddress)->~MainMenuControllerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<MainMenuControllerComponent> m_defaultArgs;
private:
	static MainMenuControllerComponentScriptComponentFormGenerator s_generator;
};

}
	