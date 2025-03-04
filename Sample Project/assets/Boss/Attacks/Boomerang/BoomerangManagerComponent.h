#pragma once

#include "DommusCore.h"



namespace Game
{
	class BoomerangManagerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::BoomerangManagerComponent>
{
	ConstructorArgs()
		:
		HalfLifeTime(2.5f),
		Velocity(10.0f),
		LowTranslationY(0.0f),
		TimeToLaunch(1.0f),
		AdditionalReturnLifeTime(0.2f)
	{}

	float HalfLifeTime;
	float Velocity;
	float LowTranslationY;
	float TimeToLaunch;
	float AdditionalReturnLifeTime;
};
#pragma pack(pop)

namespace Game
{

class BoomerangManagerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_halfLifeTime{0};
	static constexpr size_t a_velocity{1};
	static constexpr size_t a_lowTranslationY{2};
	static constexpr size_t a_timeToLaunch{3};
	static constexpr size_t a_additionalReturnLifeTime{4};
public:
	static constexpr size_t numberOfBoomerangs{2};
public:
	BoomerangManagerComponent(const DCore::ConstructorArgs<BoomerangManagerComponent>&);
	~BoomerangManagerComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
public:
	void Launch();
private:
	// Editor
	float m_halfLifeTime;
	float m_velocity;
	float m_lowTranslationY;
	float m_timeToLaunch;
	float m_adiitionalReturnLifeTime;
	// Runtime
	DCore::ComponentRef<DCore::Component> m_boomerangComponents[numberOfBoomerangs];
};

class BoomerangManagerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~BoomerangManagerComponentScriptComponentFormGenerator() = default;
private:
	BoomerangManagerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"BoomerangManagerComponent",
			DCore::ComponentId::GetId<BoomerangManagerComponent>(),
			sizeof(BoomerangManagerComponent),
			sizeof(DCore::ConstructorArgs<BoomerangManagerComponent>),
			{{DCore::AttributeName("Half Life Time"), DCore::AttributeType::Float, BoomerangManagerComponent::a_halfLifeTime},
			{DCore::AttributeName("Velocity"), DCore::AttributeType::Float, BoomerangManagerComponent::a_velocity},
			{DCore::AttributeName("Low Translation Y"), DCore::AttributeType::Float, BoomerangManagerComponent::a_lowTranslationY},
			{DCore::AttributeName("Time To Launch"), DCore::AttributeType::Float, BoomerangManagerComponent::a_timeToLaunch},
			{DCore::AttributeName("Additional Return Life Time"), DCore::AttributeType::Float, BoomerangManagerComponent::a_additionalReturnLifeTime}}, 
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) BoomerangManagerComponent(*static_cast<const DCore::ConstructorArgs<BoomerangManagerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<BoomerangManagerComponent*>(componentAddress)->~BoomerangManagerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<BoomerangManagerComponent> m_defaultArgs;
private:
	static BoomerangManagerComponentScriptComponentFormGenerator s_generator;
};

}
	
