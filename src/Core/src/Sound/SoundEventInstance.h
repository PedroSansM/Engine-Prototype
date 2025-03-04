#pragma once

#include "FixedString.h"
#include "SerializationTypes.h"

#include "fsbank.h"
#include "fmod_studio.hpp"



namespace DCore
{

class SoundEventInstance
{
public:
	using soundBankType = FMOD::Studio::Bank;
	using eventInstanceType = FMOD::Studio::EventInstance;
public:
	SoundEventInstance();
	SoundEventInstance(const DString& path);
	SoundEventInstance(const SoundEventInstance&);
	SoundEventInstance(SoundEventInstance&&) noexcept;
	~SoundEventInstance();
public:
	bool Setup();
	void Start();
	void Stop();
	bool IsPlaying();
	void Set3DAttributes(const DVec3& position, const DVec3& velocity);
public:
	const DString& GetPath() const
	{
		return m_path;
	}
	
	void Invalidate()
	{
		m_bank = nullptr;
		m_eventInstance = nullptr;
		m_path.Clear();
	}

	// Internal
	void Internal_SetBank(soundBankType* bank)
	{
		m_bank = bank;
	}

	bool Internal_IsBankNull() const
	{
		return m_bank == nullptr;
	}

	void Internal_SetPath(const char* path)
	{
		m_path = path;
	}
public:
	SoundEventInstance& operator=(const SoundEventInstance& other)
	{
		m_bank = other.m_bank;
		m_eventInstance = other.m_eventInstance;
		m_path = other.m_path;
		return *this;
	}
private:
	soundBankType* m_bank;
	eventInstanceType* m_eventInstance;
	DString m_path;
};

}
