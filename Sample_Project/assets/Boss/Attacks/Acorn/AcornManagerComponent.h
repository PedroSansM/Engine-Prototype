#pragma once

#include "DommusCore.h"



namespace Game
{
	class AcornManagerComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::AcornManagerComponent>
{
	ConstructorArgs()
		:
		TimeBetweenLaunches(1.0f)
	{}
	
	float TimeBetweenLaunches;
};
#pragma pack(pop)

namespace Game
{

class AcornManagerComponent : public DCore::ScriptComponent
{
public:
	static constexpr size_t a_timeBetweenLaunches{0};
public:
	static constexpr size_t numberOfAcornsToLaunch{3};
public:
	AcornManagerComponent(const DCore::ConstructorArgs<AcornManagerComponent>&);
	~AcornManagerComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
	virtual void Update(float deltaTime) override;
public:
	void LaunchAcorns();
private:
	// Editor
	float m_timeBetweenLaunches;
	// Runtime
	float m_currentLaunchTime;
	size_t m_numberOfLaunches;
	size_t m_launchOrder[numberOfAcornsToLaunch];
	DCore::ComponentRef<Component> m_acornComponents[numberOfAcornsToLaunch];
};

class AcornManagerComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~AcornManagerComponentScriptComponentFormGenerator() = default;
private:
	AcornManagerComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"AcornManagerComponent",
			DCore::ComponentId::GetId<AcornManagerComponent>(),
			sizeof(AcornManagerComponent),
			sizeof(DCore::ConstructorArgs<AcornManagerComponent>),
			{{DCore::AttributeName("Time Between Acorn Launches"), DCore::AttributeType::Float, AcornManagerComponent::a_timeBetweenLaunches}}, 
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) AcornManagerComponent(*static_cast<const DCore::ConstructorArgs<AcornManagerComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<AcornManagerComponent*>(componentAddress)->~AcornManagerComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<AcornManagerComponent> m_defaultArgs;
private:
	static AcornManagerComponentScriptComponentFormGenerator s_generator;
};

}
	
