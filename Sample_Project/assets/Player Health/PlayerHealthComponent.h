#pragma once

#include "DommusCore.h"



namespace Game
{
	class PlayerHealthComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::PlayerHealthComponent>
{
	ConstructorArgs()
	{}
};
#pragma pack(pop)

namespace Game
{

class PlayerHealthComponent : public DCore::ScriptComponent
{
public:
	PlayerHealthComponent(const DCore::ConstructorArgs<PlayerHealthComponent>&)
	{}
	~PlayerHealthComponent() = default;
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
	void DecreaseHealth();
public:
	DCore::DUInt GetCurrentHealth() const
	{
		return m_currentHeath;
	}
private:
	DCore::ComponentRef<DCore::SpriteComponent> m_spriteComponent;
	DCore::DUInt m_currentHeath;
	float m_currentBlinkPeriod;
};

class PlayerHealthComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~PlayerHealthComponentScriptComponentFormGenerator() = default;
private:
	PlayerHealthComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"PlayerHealthComponent",
			DCore::ComponentId::GetId<PlayerHealthComponent>(),
			sizeof(PlayerHealthComponent),
			sizeof(DCore::ConstructorArgs<PlayerHealthComponent>),
			{}, // Serialized attributes
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) PlayerHealthComponent(*static_cast<const DCore::ConstructorArgs<PlayerHealthComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<PlayerHealthComponent*>(componentAddress)->~PlayerHealthComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<PlayerHealthComponent> m_defaultArgs;
private:
	static PlayerHealthComponentScriptComponentFormGenerator s_generator;
};

}
	