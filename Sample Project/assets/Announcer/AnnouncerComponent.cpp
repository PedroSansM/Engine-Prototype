#include "AnnouncerComponent.h"



namespace Game
{

AnnouncerComponentScriptComponentFormGenerator AnnouncerComponentScriptComponentFormGenerator::s_generator;

AnnouncerComponent::AnnouncerComponent(const DCore::ConstructorArgs<AnnouncerComponent>& args)
	:
	m_readySound(args.ReadySound),
	m_beginSound(args.BeginSound),
	m_delayToStartReadySound(args.DelayToStartReadySound),
	m_delayToStartBeginSound(args.DelayToStartBeginSound)
{}

void* AnnouncerComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_readySound:
		return &m_readySound;
	case a_beginSound:
		return &m_beginSound;
	case a_delayToStartReadySound:
		return &m_delayToStartReadySound;
	case a_delayToStartBeginSound:
		return &m_delayToStartBeginSound;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void AnnouncerComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_readySound:
		m_readySound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;
	case a_beginSound:
		m_beginSound = *static_cast<DCore::DSoundEventInstance*>(newValue);
		return;
	case a_delayToStartReadySound:
		m_delayToStartReadySound = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	case a_delayToStartBeginSound:
	 	m_delayToStartBeginSound = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	default:
		return;
	}
}

void AnnouncerComponent::Start()
{
	DASSERT_K(m_readySound.Setup());
	DASSERT_K(m_beginSound.Setup());
	m_currentDelayToStartReadySound = m_delayToStartReadySound;
	m_currentDelayToStartBeginSound = m_delayToStartBeginSound;
}

void AnnouncerComponent::Update(float deltaTime)
{
	if (m_currentDelayToStartReadySound > 0.0f)
	{
		m_currentDelayToStartReadySound -= deltaTime;
		if (m_currentDelayToStartReadySound <= 0.0f)
		{
			m_readySound.Start();
		}
		return;
	}
	if (m_currentDelayToStartBeginSound > 0.0f)
	{
		m_currentDelayToStartBeginSound -= deltaTime;
		if (m_currentDelayToStartBeginSound <= 0.0f)
		{
			m_beginSound.Start();
		}
	}
}

}