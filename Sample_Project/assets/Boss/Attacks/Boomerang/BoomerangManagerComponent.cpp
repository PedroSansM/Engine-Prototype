#include "BoomerangManagerComponent.h"
#include "BoomerangComponent.h"

#include "Log.h"



namespace Game
{

BoomerangManagerComponentScriptComponentFormGenerator BoomerangManagerComponentScriptComponentFormGenerator::s_generator;

BoomerangManagerComponent::BoomerangManagerComponent(const DCore::ConstructorArgs<BoomerangManagerComponent>& args)
	:
	m_halfLifeTime(args.HalfLifeTime),
	m_velocity(args.Velocity),
	m_lowTranslationY(args.LowTranslationY),
	m_timeToLaunch(args.TimeToLaunch),
	m_adiitionalReturnLifeTime(args.AdditionalReturnLifeTime)
{}

void* BoomerangManagerComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_halfLifeTime:
		return &m_halfLifeTime;	
	case a_velocity:
		return &m_velocity;
	case a_lowTranslationY:
		return &m_lowTranslationY;
	case a_timeToLaunch:
		return &m_timeToLaunch;
	case a_additionalReturnLifeTime:
		return &m_adiitionalReturnLifeTime;
	default:
		return nullptr;
	}
	return nullptr;
}

void BoomerangManagerComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_halfLifeTime:
		m_halfLifeTime = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_velocity:
		m_velocity = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_lowTranslationY:
		m_lowTranslationY = *static_cast<float*>(newValue);
		return;
	case a_timeToLaunch:
		m_timeToLaunch = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_additionalReturnLifeTime:
		m_adiitionalReturnLifeTime = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	default:
		return;
	}
}

void BoomerangManagerComponent::Start()
{
	size_t currentBoomerangIndex(0);
	const DCore::ComponentIdType boomerangComponentId(DCore::ComponentId::GetId<BoomerangComponent>());
	IterateOnEntitiesWithComponents
	(
		&boomerangComponentId, 1, 
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> boomerangComponentRef) -> bool
		{
			m_boomerangComponents[currentBoomerangIndex] = boomerangComponentRef;
			BoomerangComponent* boomerangComponent(static_cast<BoomerangComponent*>(boomerangComponentRef.GetRawComponent()));
			boomerangComponent->SetHalfLifeTime(m_halfLifeTime);
			boomerangComponent->SetVelocity(m_velocity);
			boomerangComponent->SetLowTranslationY(m_lowTranslationY);
			boomerangComponent->SetTimeToLaunch(m_timeToLaunch);
			boomerangComponent->SetAdditionalReturnLifeTime(m_adiitionalReturnLifeTime);
			return currentBoomerangIndex++ >= numberOfBoomerangs;
		}
	);
}

void BoomerangManagerComponent::Launch()
{
	for (size_t i(0); i < numberOfBoomerangs; i++)
	{
		BoomerangComponent* boomerangComponent(static_cast<BoomerangComponent*>(m_boomerangComponents[i].GetRawComponent()));
		if (!boomerangComponent->IsFree())
		{
			continue;;
		}
		boomerangComponent->Enable();
		return;
	}
}

}
