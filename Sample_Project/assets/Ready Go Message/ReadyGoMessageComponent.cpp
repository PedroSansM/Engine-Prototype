#include "ReadyGoMessageComponent.h"



namespace Game
{

ReadyGoMessageComponentScriptComponentFormGenerator ReadyGoMessageComponentScriptComponentFormGenerator::s_generator;

ReadyGoMessageComponent::ReadyGoMessageComponent(const DCore::ConstructorArgs<ReadyGoMessageComponent>& args)
	:
	m_presentationDelay(args.PresentationDelay)
{}

void* ReadyGoMessageComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_presentationDelay:
		return &m_presentationDelay;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void ReadyGoMessageComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_presentationDelay:
		m_presentationDelay = std::max(*static_cast<float*>(newValue), 0.0f);
		return;
	default:
		return;
	}
}

void ReadyGoMessageComponent::Start()
{
	m_currentPresentationDelay = m_presentationDelay;
	auto [spriteComponent, asmComponent] = m_entityRef.GetComponents<DCore::SpriteComponent, DCore::AnimationStateMachineComponent>();
	m_spriteComponent = spriteComponent;
	m_asmComponent = asmComponent;
	DASSERT_E(m_spriteComponent.IsValid() && m_asmComponent.IsValid());
	m_spriteComponent.SetEnabled(false);
	DASSERT_K(m_asmComponent.TryGetParameterIndexWithName<DCore::ParameterType::Logic>("Display", m_displayAnimationParameter));
}

void ReadyGoMessageComponent::Update(float deltaTime)
{
	if (m_currentPresentationDelay > 0.0f)
	{
		m_currentPresentationDelay -= deltaTime;
		if (m_currentPresentationDelay <= 0.0f)
		{
			m_spriteComponent.SetEnabled(true);
			m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_displayAnimationParameter, DCore::LogicParameter{ true });
		}
	}
}

void ReadyGoMessageComponent::OnMetachannelEvent(size_t eventId)
{
	constexpr size_t displayEnd{0};
	switch (eventId)
	{
	case displayEnd:
		m_spriteComponent.SetEnabled(false);
		m_asmComponent.SetParameterValue<DCore::ParameterType::Logic>(m_displayAnimationParameter, DCore::LogicParameter{ false });
		return;
	default:
		break;
	}
}

}