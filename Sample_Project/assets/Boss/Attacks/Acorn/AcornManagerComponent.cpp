#include "AcornManagerComponent.h"
#include "AcornComponent.h"

#include <random>



namespace Game
{

AcornManagerComponentScriptComponentFormGenerator AcornManagerComponentScriptComponentFormGenerator::s_generator;

AcornManagerComponent::AcornManagerComponent(const DCore::ConstructorArgs<AcornManagerComponent>& args)
	:
	m_timeBetweenLaunches(args.TimeBetweenLaunches)
{}

void* AcornManagerComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_timeBetweenLaunches:
		return &m_timeBetweenLaunches;
	default:
		return nullptr;
	}
	return nullptr;
}

void AcornManagerComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_timeBetweenLaunches:
		m_timeBetweenLaunches = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	default:
		return;
	}
}

void AcornManagerComponent::Start()
{
	m_currentLaunchTime = m_timeBetweenLaunches;
	m_numberOfLaunches = numberOfAcornsToLaunch;
	size_t acornComponentIndex(0);
	const DCore::ComponentIdType acornComponentId(DCore::ComponentId::GetId<AcornComponent>());
	IterateOnEntitiesWithComponents
	(
		&acornComponentId, 1,
		[&](DCore::Entity, DCore::ComponentRef<Component> acornComponent) -> bool
		{
			m_acornComponents[acornComponentIndex++] = acornComponent;
			return acornComponentIndex >= numberOfAcornsToLaunch;
		}
	);
}

void AcornManagerComponent::Update(float deltaTime)
{
	if (m_numberOfLaunches >= numberOfAcornsToLaunch)
	{
		return;
	}
	m_currentLaunchTime -= deltaTime;
	if (m_currentLaunchTime <= 0.0f)
	{
		AcornComponent* acornComponent(static_cast<AcornComponent*>(m_acornComponents[m_launchOrder[m_numberOfLaunches++]].GetRawComponent()));
		acornComponent->Launch();
		m_currentLaunchTime = m_timeBetweenLaunches;
	}
}

void AcornManagerComponent::LaunchAcorns()
{
	m_currentLaunchTime = m_timeBetweenLaunches;
	m_numberOfLaunches = 0;
	for (size_t i(0); i < 3; i++)
	{
		AcornComponent* acornComponent(static_cast<AcornComponent*>(m_acornComponents[i].GetRawComponent()));
		acornComponent->Enable();	
	}
	constexpr size_t upMiddleDown(0);
	constexpr size_t middleUpDown(1);
	constexpr size_t downMiddleUp(2);
	constexpr size_t upDownMiddle(3);
	static std::mt19937 randomGenerator(std::random_device{}());
	static std::discrete_distribution distribution({1, 1, 1, 1});
	const size_t acornOrder(distribution(randomGenerator));
	switch (acornOrder) 
	{
	case upMiddleDown:
		m_launchOrder[0] = 0;
		m_launchOrder[1] = 1;
		m_launchOrder[2] = 2;
		return;
	case middleUpDown:
		m_launchOrder[0] = 1;
		m_launchOrder[1] = 0;
		m_launchOrder[2] = 2;
		return;
	case downMiddleUp:
		m_launchOrder[0] = 2;
		m_launchOrder[1] = 1;
		m_launchOrder[2] = 0;
		return;
	case upDownMiddle:
		m_launchOrder[0] = 0;
		m_launchOrder[1] = 2;
		m_launchOrder[2] = 1;
		return;
	}
}

}
