#include "MainMenuControllerComponent.h"
#include "GameManagerComponent.h"



namespace Game
{

MainMenuControllerComponentScriptComponentFormGenerator MainMenuControllerComponentScriptComponentFormGenerator::s_generator;

void MainMenuControllerComponent::Start()
{
	m_enabled = true;
	m_selectedOptionIndex = startOptionIndex;
	const DCore::ComponentIdType gameManagerComponentId(DCore::ComponentId::GetId<GameManagerComponent>());
	IterateOnEntitiesWithComponents(
		&gameManagerComponentId, 1,
		[&](DCore::Entity, DCore::ComponentRef<DCore::Component> component) -> bool
		{
			m_gameManagerComponent = component;
			DASSERT_E(m_gameManagerComponent.IsValid());
			return true;
		});
	m_entityRef.IterateOnChildren(
		[&](DCore::EntityRef child) -> bool
		{
			DCore::DString name;
			child.GetName(name);
			if (name == "Start")
			{
				m_optionSpriteComponents[startOptionIndex] = child.GetComponents<DCore::SpriteComponent>();
				DASSERT_E(m_optionSpriteComponents[startOptionIndex].IsValid());
				return false;
			}
			if (name == "Controls")
			{
				m_optionSpriteComponents[controlsOptionIndex] = child.GetComponents<DCore::SpriteComponent>();
				DASSERT_E(m_optionSpriteComponents[controlsOptionIndex].IsValid());
				return false;
			}
			m_backgroundElementSpriteComponents.PushBack(child.GetComponents<DCore::SpriteComponent>());
			return false;
		});
#ifndef NDEBUG
	for (DCore::ComponentRef<DCore::SpriteComponent> spriteComponent : m_backgroundElementSpriteComponents)
	{
		DASSERT_E(spriteComponent.IsValid());
	}
#endif
}

void MainMenuControllerComponent::Update(float)
{
	if (!m_enabled)
	{
		return;
	}
	if (m_runtime->KeyPressedThisFrame(DCore::DKey::Z))
	{
		if (m_selectedOptionIndex == startOptionIndex)
		{
			GameManagerComponent* gameManagerComponent(static_cast<GameManagerComponent*>(m_gameManagerComponent.GetRawComponent()));
			gameManagerComponent->StartGame();
			for (size_t i(0); i < numberOfOptions; i++)
			{
				m_optionSpriteComponents[i].SetEnabled(false);
			}
			for (DCore::ComponentRef<DCore::SpriteComponent> spriteComponent : m_backgroundElementSpriteComponents)
			{
				spriteComponent.SetEnabled(false);
			}
			m_enabled = false;
		}
	}
	if (m_runtime->KeyPressedThisFrame(DCore::DKey::ArrowUp))
	{
		if (m_selectedOptionIndex == startOptionIndex)
		{
			m_selectedOptionIndex = controlsOptionIndex;
		}
		else
		{
			m_selectedOptionIndex--;
		}
	}
	if (m_runtime->KeyPressedThisFrame(DCore::DKey::ArrowDown))
	{
		if (m_selectedOptionIndex == controlsOptionIndex)
		{
			m_selectedOptionIndex = startOptionIndex;
		}
		else
		{
			m_selectedOptionIndex++;
		}
	}
	HandleOptionsHighlight();
}

void MainMenuControllerComponent::Enable()
{
	m_selectedOptionIndex = startOptionIndex;
	m_enabled = true;
	for (size_t i(0); i < numberOfOptions; i++)
	{
		m_optionSpriteComponents[i].SetEnabled(true);
	}
	for (DCore::ComponentRef<DCore::SpriteComponent> spriteComponent : m_backgroundElementSpriteComponents)
	{
		spriteComponent.SetEnabled(true);
	}
}

void MainMenuControllerComponent::HandleOptionsHighlight()
{
	for (size_t i(0); i < numberOfOptions; i++)
	{
		if (i == m_selectedOptionIndex)
		{
			m_optionSpriteComponents[i].SetTintColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			continue;
		}
		m_optionSpriteComponents[i].SetTintColor({ 0.3f, 0.3f, 0.3f, 0.3f });
	}
}

}