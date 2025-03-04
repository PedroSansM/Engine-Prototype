#pragma once

#include "DommusCore.h"



namespace Game
{
	class SeedEnemiesLauncherComponent;
}

#pragma pack(push, 1)
template <>
struct DCore::ConstructorArgs<Game::SeedEnemiesLauncherComponent>
{
	ConstructorArgs()
		:
		MaxSeedSpawnDistances(10.0f, 10.0f)
	{}

	DCore::DVec2 MaxSeedSpawnDistances;
};
#pragma pack(pop)

namespace Game
{

class SeedEnemiesLauncherComponent : public DCore::ScriptComponent
{
public:
	using seedTupleType = std::tuple<DCore::ComponentRef<DCore::Component>, DCore::ComponentRef<DCore::TransformComponent>>;
	using seedTupleContainerType = std::vector<seedTupleType>;
public:
	static constexpr size_t a_maxSeedSpawnDistances{ 0 };
public:
	SeedEnemiesLauncherComponent(const DCore::ConstructorArgs<SeedEnemiesLauncherComponent>&);
	~SeedEnemiesLauncherComponent() = default;
public:
	virtual void* GetAttributePtr(DCore::AttributeIdType attributeId) override;
	virtual void OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint) override;
public:
	virtual void Start() override;
public:
	void LaunchSeeds();
private:
	// Editor
	DCore::DVec2 m_maxSeedSpawnDistances;
	// Runtime
	DCore::DVec2 m_referencePosition;
	seedTupleContainerType m_seedComponents;
	DCore::ComponentRef<DCore::TransformComponent> m_transformComponent;
};

class SeedEnemiesLauncherComponentScriptComponentFormGenerator : private DCore::ScriptComponentFormGenerator
{
public:
	~SeedEnemiesLauncherComponentScriptComponentFormGenerator() = default;
private:
	SeedEnemiesLauncherComponentScriptComponentFormGenerator()
		:
		DCore::ScriptComponentFormGenerator
		(
			"SeedEnemiesLauncherComponent",
			DCore::ComponentId::GetId<SeedEnemiesLauncherComponent>(),
			sizeof(SeedEnemiesLauncherComponent),
			sizeof(DCore::ConstructorArgs<SeedEnemiesLauncherComponent>),
			{ { DCore::AttributeName("Max Seed Spawn Distances#X#Y"), DCore::AttributeType::Vector2 , SeedEnemiesLauncherComponent::a_maxSeedSpawnDistances }},
			[](void* componentAddress, const void* args) -> void // Constructor
			{
				new (componentAddress) SeedEnemiesLauncherComponent(*static_cast<const DCore::ConstructorArgs<SeedEnemiesLauncherComponent>*>(args));
			},
			[](void* componentAddress) -> void // Destructor
			{
				static_cast<SeedEnemiesLauncherComponent*>(componentAddress)->~SeedEnemiesLauncherComponent();
			},
			&m_defaultArgs
		)
	{}
private:
	DCore::ConstructorArgs<SeedEnemiesLauncherComponent> m_defaultArgs;
private:
	static SeedEnemiesLauncherComponentScriptComponentFormGenerator s_generator;
};

}
	