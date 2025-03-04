#pragma once

#include "UUID.h"
#include "ReadWriteLockGuard.h"
#include "AspectRatioUtilities.h"

#include <string>



namespace DCore
{

class GlobalConfig
{
	friend class ReadWriteLockGuard;
public:
	using stringType = std::string;
public:
	GlobalConfig(const GlobalConfig&) = delete;
	GlobalConfig(GlobalConfig&&) = delete;
	~GlobalConfig() = default;
public:
	static GlobalConfig& Get()
	{
		static GlobalConfig globalConfig;
		return globalConfig;
	}
public:
	UUIDType GetStartingSceneUUID() const
	{
		return static_cast<UUIDType>(m_startingSceneUUIDString);
	}
	
	void SetStaringSceneUUID(const UUIDType& uuid)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
		m_startingSceneUUIDString = static_cast<stringType>(uuid);
	}

	bool IsStartingSceneDefined() const
	{
		return !m_startingSceneUUIDString.empty();
	}

	AspectRatio GetTargetAspectRatio() const
	{
		return m_targetAspectRatio;
	}
	
	void SetTargetAspectRatio(AspectRatio value)
	{
		ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
		m_targetAspectRatio = value;
	}
private:
	GlobalConfig()
		:
		m_targetAspectRatio(AspectRatio::FreeAspect) 
	{}
private:
	stringType m_startingSceneUUIDString;
	AspectRatio m_targetAspectRatio;
	LockData m_lockData;
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
