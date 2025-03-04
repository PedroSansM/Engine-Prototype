#include "GlobalConfigurationSerializer.h"
#include "ProgramContext.h"
#include "Path.h"
#include "Log.h"

#include "yaml-cpp/yaml.h"

#include <cstdio>
#include <fstream>
#include <filesystem>



namespace DEditor
{

static const char* s_configFileName{"Config.yaml"};
static const char* s_startingSceneKey{"Starting Scene"};
static const char* s_aspectRatioKey{"Target Aspect Ratio"};
static const char* s_banksKey{"Banks"};
YAML::Node s_configNode;

GlobalConfigurationSerializer::GlobalConfigurationSerializer()
{
	std::filesystem::path configFilePath(ProgramContext::Get().GetProjectRootDirectoryPath() / s_configFileName);
	std::ofstream ostream(configFilePath, std::ios_base::out | std::ios_base::app);
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	s_configNode = YAML::LoadFile(configFilePath.string());
	if (!s_configNode[s_startingSceneKey])
	{
		s_configNode[s_startingSceneKey] = "";
	}
	if (!s_configNode[s_aspectRatioKey])
	{
		s_configNode[s_aspectRatioKey] = DCore::AspectRatioUtilities::Get().GetAspectRatioString(DCore::AspectRatio::FreeAspect);
	}
	if (const stringType startingSceneUUIDString(s_configNode[s_startingSceneKey].as<stringType>()); !startingSceneUUIDString.empty())
	{
		DCore::GlobalConfig::Get().SetStaringSceneUUID(static_cast<uuidType>(startingSceneUUIDString));
	}
	UpdateSoundBanks();
	DCore::GlobalConfig::Get().SetTargetAspectRatio(DCore::AspectRatioUtilities::Get().GetAspectRatioFromString(s_configNode[s_aspectRatioKey].as<stringType>().c_str()));
}

void GlobalConfigurationSerializer::SetStartingSceneUUID(const uuidType& uuid)
{
	s_configNode[s_startingSceneKey] = static_cast<stringType>(uuid);
	DCore::GlobalConfig::Get().SetStaringSceneUUID(uuid);
}

void GlobalConfigurationSerializer::SetTargetAspectRatio(DCore::AspectRatio aspectRatio)
{
	s_configNode[s_aspectRatioKey] = DCore::AspectRatioUtilities::Get().GetAspectRatioString(aspectRatio);
	DCore::GlobalConfig::Get().SetTargetAspectRatio(aspectRatio);
}

void GlobalConfigurationSerializer::UpdateSoundBanks()
{
	m_banksNames.clear();
	DCore::Sound::Get().UnloadAllBanks();
	try 
	{
		for (const auto& dirEntry : std::filesystem::directory_iterator(ProgramContext::Get().GetSoundBacksDirectoryPath()))
		{
			if (dirEntry.is_directory())
			{
				continue;
			}
			DCore::ReturnError error;
			DCore::Sound::Get().AddBank(dirEntry.path().c_str(), error);
			if (!error.Ok)
			{
				Log::Get().TerminalLog("Error while updating sound banks: %s", error.Message.Data());
				Log::Get().ConsoleLog(LogLevel::Warning, "Error while updating sound banks: %s", error.Message.Data());
				continue;
			}
			Log::Get().TerminalLog("Bank at path loaded with success: %s.", dirEntry.path().c_str());
			Log::Get().ConsoleLog(LogLevel::Default, "Bank at path loaded with success: %s.", dirEntry.path().c_str());
			m_banksNames.push_back(dirEntry.path().stem().string());
		}
	} 
	catch (std::exception& e)
	{
		Log::Get().TerminalLog("Error while updating sound banks: %s.", e.what());
		Log::Get().ConsoleLog(LogLevel::Warning, "Error while updating sound banks: %s.", e.what());
	}
}

void GlobalConfigurationSerializer::Save()
{
	YAML::Emitter emitter;
	emitter << s_configNode;
	DASSERT_E(emitter.good());
	std::filesystem::path configFilePath(ProgramContext::Get().GetProjectRootDirectoryPath() / s_configFileName);
	std::ofstream ostream(configFilePath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

}
