#include "MusicManagerComponent.h"



namespace Game
{

MusicManagerComponentScriptComponentFormGenerator MusicManagerComponentScriptComponentFormGenerator::s_generator;

MusicManagerComponent::MusicManagerComponent(const DCore::ConstructorArgs<MusicManagerComponent>& args)
	:
	m_mainMenuSound(args.MainMenuMusic),
	m_bossSound(args.BossMusic)
{}

void* MusicManagerComponent::GetAttributePtr(DCore::AttributeIdType attributeId)
{
	switch (attributeId)
	{
	case a_mainMenuSound:
		return &m_mainMenuSound;
	case a_bossSound:
		return &m_bossSound;
	default:
		break;
	}
	DASSERT_E(false);
	return nullptr;
}

void MusicManagerComponent::OnAttributeChange(DCore::AttributeIdType attributeId, void* newValue, DCore::AttributeType typeHint)
{
	switch (attributeId)
	{
	case a_mainMenuSound:
		m_mainMenuSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	case a_bossSound:
		m_bossSound = *static_cast<DCore::SoundEventInstance*>(newValue);
		return;
	default:
		break;
	}
}

void MusicManagerComponent::Start()
{
	DASSERT_K(m_mainMenuSound.Setup());
	DASSERT_K(m_bossSound.Setup());
	m_mainMenuSound.Start();
}

void MusicManagerComponent::PlayMainMenuMusic()
{
	m_mainMenuSound.Start();
}

void MusicManagerComponent::StopMainMenuMusic()
{
	m_mainMenuSound.Stop();
}

void MusicManagerComponent::PlayBossMusic()
{
	m_bossSound.Start();
}

void MusicManagerComponent::StopBossMusic()
{
	m_bossSound.Stop();
}

}