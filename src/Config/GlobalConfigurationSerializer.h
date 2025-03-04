#pragma once

#include "DommusCore.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <functional>
#include <string>



namespace DEditor
{

class GlobalConfigurationSerializer
{
public:
	using uuidType = DCore::UUIDType;
	using stringType = std::string;
	using pathType = std::filesystem::path;
	using bankNameContainerType = std::vector<stringType>;
public:
	GlobalConfigurationSerializer(const GlobalConfigurationSerializer&) = delete;
	GlobalConfigurationSerializer(GlobalConfigurationSerializer&&) = delete;
	~GlobalConfigurationSerializer() = default;
public:
	static GlobalConfigurationSerializer& Get()
	{
		static GlobalConfigurationSerializer instance;
		return instance;
	}
public:
	void SetStartingSceneUUID(const uuidType&);
	void SetTargetAspectRatio(DCore::AspectRatio);
	void UpdateSoundBanks();
	void Save();
public:
	template <class Func>
	void IterateOnBanksNames(Func function)
	{
		for (const stringType& name : m_banksNames)
		{
			if (std::invoke(function, name))
			{
				return;
			}
		}
	}
private:
	GlobalConfigurationSerializer();
private:
	bankNameContainerType m_banksNames;
};

}
