#pragma once

#include "ReturnError.h"
#include "ReadWriteLockGuard.h"

#include "fmod_studio.hpp"

#include <filesystem>



namespace DCore
{

class Sound
{
	friend class ReadWriteLockGuard;
public:
	using pathType = std::filesystem::path;
	using eventInstanceType = FMOD::Studio::EventInstance;
	using fmodSystemType = FMOD::Studio::System;
	using bankType = FMOD::Studio::Bank;
	using bankContainerType = std::vector<bankType*>;
public:
	Sound(const Sound&) = delete;
	Sound(Sound&&) = delete;
	~Sound() = default;
public:
	static Sound& Get()
	{
		static Sound instance;
		return instance;
	}
public:
	void AddBank(const pathType&, ReturnError&);
	eventInstanceType* TryCreateEventInstance(const char* path);
	void UnloadAllBanks();
	void Update3DAudioListener();
	void Update();
public:
	template <class Func>
	void IterateOnLoadedBanks(Func function)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
		for (bankType* bank : m_loadedBanks)
		{
			if (std::invoke(function, bank))
			{
				return;
			}
		}
	}
private:
	Sound();	
private:
	fmodSystemType* m_system;
	bankContainerType m_loadedBanks;
	LockData m_lockData;
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
