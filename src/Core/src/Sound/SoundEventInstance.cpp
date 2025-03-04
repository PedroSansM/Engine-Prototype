#include "SoundEventInstance.h"
#include "ReadWriteLockGuard.h"
#include "Sound.h"

#include "fsbank.h"



namespace DCore
{

SoundEventInstance::SoundEventInstance()
	:
	m_bank(nullptr),
	m_eventInstance(nullptr)
{}

SoundEventInstance::SoundEventInstance(const DString& path)
	:
	m_bank(nullptr),
	m_eventInstance(nullptr),
	m_path(path)
{}

SoundEventInstance::SoundEventInstance(const SoundEventInstance& other)
	:
	m_bank(other.m_bank),
	m_eventInstance(other.m_eventInstance),
	m_path(other.m_path)
{}

SoundEventInstance::SoundEventInstance(SoundEventInstance&& other) noexcept
	:
	m_bank(other.m_bank),
	m_eventInstance(other.m_eventInstance),
	m_path(std::move(other.m_path))
{
	other.m_bank = nullptr;
	other.m_eventInstance = nullptr;
}

SoundEventInstance::~SoundEventInstance()
{
	if (m_bank != nullptr)
	{
		Stop();
		m_bank->unloadSampleData();
	}
}

bool SoundEventInstance::Setup()
{
	if (m_path.Empty() || m_bank == nullptr)
	{
		return false;
	}
	m_bank->loadSampleData();
	m_eventInstance = Sound::Get().TryCreateEventInstance(m_path.Data());
	return m_eventInstance != nullptr;
}

void SoundEventInstance::Start()
{
	if (m_eventInstance == nullptr)
	{
		return;
	}
	FMOD_RESULT result(m_eventInstance->start());
	DASSERT_E(result == FMOD_OK);
}

void SoundEventInstance::Stop()
{
	if (m_eventInstance == nullptr)
	{
		return;
	}
	FMOD_RESULT result(m_eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT));
	DASSERT_E(result == FMOD_OK);
}

bool SoundEventInstance::IsPlaying()
{
	if (m_eventInstance == nullptr)
	{
		return false;
	}
	FMOD_STUDIO_PLAYBACK_STATE state;
	FMOD_RESULT result(m_eventInstance->getPlaybackState(&state));
	DASSERT_E(result == FMOD_OK);
	return state == FMOD_STUDIO_PLAYBACK_PLAYING;
}

void SoundEventInstance::Set3DAttributes(const DVec3& position, const DVec3& velocity)
{
	if (m_eventInstance == nullptr)
	{
		return;
	}
	FMOD_3D_ATTRIBUTES attributes;
	attributes.position = { position.x, position.y, position.z };
	attributes.velocity = { velocity.x, velocity.y, velocity.z };
	attributes.forward = { 0.0f, 0.0f, 1.0 };
	attributes.up = { 0.0f, 1.0f, 0.0f };
	FMOD_RESULT result(m_eventInstance->set3DAttributes(&attributes));
	DASSERT_E(result == FMOD_OK);
}

}

