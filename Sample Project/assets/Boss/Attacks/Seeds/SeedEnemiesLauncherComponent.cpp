#include "SeedEnemiesLauncherComponent.h"
#include "SeedComponent.h"

#include <random>



namespace Game
{

static std::mt19937 s_randomGenerator(std::random_device{}());

SeedEnemiesLauncherComponentScriptComponentFormGenerator SeedEnemiesLauncherComponentScriptComponentFormGenerator::s_generator;

SeedEnemiesLauncherComponent::SeedEnemiesLauncherComponent(const DCore::ConstructorArgs<SeedEnemiesLauncherComponent>& args)
	:
	m_maxSeedSpawnDistances(args.MaxSeedSpawnDistances)
{}

void* SeedEnemiesLauncherComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_maxSeedSpawnDistances:
		return &m_maxSeedSpawnDistances;
	default:
		break;
	}
	return nullptr;
}

void SeedEnemiesLauncherComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_maxSeedSpawnDistances:
		m_maxSeedSpawnDistances.x = std::max((*static_cast<DCore::DVec2*>(newValue)).x, 0.0f);
		m_maxSeedSpawnDistances.y = std::max((*static_cast<DCore::DVec2*>(newValue)).y, 0.0f);
		return;
	default:
		return;
	}
}

void SeedEnemiesLauncherComponent::Start()
{
	m_transformComponent = m_entityRef.GetComponents<DCore::TransformComponent>();
	const DCore::ComponentIdType seedComponentId(DCore::ComponentId::GetId<SeedComponent>());
	IterateOnEntitiesWithComponents(
		&seedComponentId, 1,
		[&](DCore::Entity entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			DCore::EntityRef entityRef(entity, m_entityRef.GetSceneRef());
			DCore::EntityRef parent;
			DASSERT_K(entityRef.TryGetParent(parent));
			m_seedComponents.emplace_back(component, parent.GetComponents<DCore::TransformComponent>());
			return false;
		});
}

void SeedEnemiesLauncherComponent::LaunchSeeds()
{
	const DCore::DVec2 startPosition(m_transformComponent.GetTranslation());
	for (seedTupleType& seedTuple : m_seedComponents)
	{
		const DCore::DVec2 seedSpawnPosition(
			startPosition
			+ DCore::DVec2(
				std::generate_canonical<float, 10>(s_randomGenerator) * m_maxSeedSpawnDistances.x,
				std::generate_canonical<float, 10>(s_randomGenerator) * m_maxSeedSpawnDistances.y));
		DCore::ComponentRef<DCore::Component> seedComponentRef(std::get<0>(seedTuple));
		DCore::ComponentRef<DCore::TransformComponent> transformComponent(std::get<1>(seedTuple));
		transformComponent.SetTranslation({ seedSpawnPosition.x, seedSpawnPosition.y, 0.0f });
		SeedComponent* seedComponent(static_cast<SeedComponent*>(seedComponentRef.GetRawComponent()));
		seedComponent->BeginFall();
	}
}

}