#include "SceneManager.h"
#include "Path.h"
#include "ProgramContext.h"
#include "Log.h"
#include "SceneSerialization.h"

#include "yaml-cpp/yaml.h"

#include <fstream>



namespace DEditor
{

static YAML::Node s_scenesNode;							// UUID -> name + path
static const char* s_scenesDirectory = "scene";
static const char* s_scenesFileName = "scenes.dscenes";
static const char* s_sceneFileExtension = ".dscene";
static const char* s_sceneThumbnailFileExtension = ".dtscene";
static const char* s_uuidKey = "UUID";
static const char* s_nameKey = "Name";
static const char* s_pathKey = "Path";
static const char* s_sceneKey = "Scene";

SceneManager::SceneManager()
{
	const std::filesystem::path scenesPath(GetSceneDirectoryPath() / s_scenesFileName);
	std::ofstream ostream(scenesPath, std::ios_base::out | std::ios_base::app);
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	s_scenesNode = YAML::LoadFile(scenesPath.string());
	DCore::SceneLoader::Get().SetLoadSceneFunc(
		[&](const stringType& sceneName) -> sceneRefType 
		{
			sceneRefType scene;
			LoadSceneWithName(sceneName, &scene); 
			return scene;
		});
}

void SceneManager::CreateScene(const stringType& sceneName, const pathType& thumbnailDirectory)
{
	for (YAML::const_iterator it(s_scenesNode.begin()); it != s_scenesNode.end(); it++)
	{
		if (sceneName == it->second[s_nameKey].as<stringType>())
		{
			Log::Get().TerminalLog("Scene with name ", sceneName.c_str(), " is already created.");
			Log::Get().ConsoleLog(LogLevel::Error, "Scene with name %s is already created.", sceneName.c_str());
			return;
		}
	}
	DCore::Scene scene(sceneName);
	DCore::DString sceneNameWithFileExtension(sceneName);
	sceneNameWithFileExtension.Append(s_sceneFileExtension);
	DCore::UUIDType sceneUUID;
	DCore::UUIDGenerator::Get().GenerateUUID(sceneUUID);
	std::filesystem::path scenePath(GetSceneDirectoryPath() / sceneNameWithFileExtension.Data());
	DCore::SceneRef sceneRef(DCore::AssetManager::Get().LoadScene(sceneUUID, std::move(scene)));
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	DCore::ReturnError error(SceneSerialization::Get().SerializeScene(scenePath, sceneRef));
	if (!error.Ok)
	{
		Log::Get().TerminalLog(error.Message);
		return;
	}
	YAML::Node sceneInfoNode;
	sceneInfoNode[s_nameKey] = sceneName.c_str();
	sceneInfoNode[s_pathKey] = Path::Get().MakePathRelativeToAssetsDirectory(scenePath).string().c_str();
	const stringType uuidString((std::string)sceneUUID);
	s_scenesNode[uuidString] = sceneInfoNode;
	pathType thumbnailPath(thumbnailDirectory / sceneName);
	thumbnailPath += s_sceneThumbnailFileExtension;
	GenerateSceneThumbnail(uuidString, thumbnailPath);
	SaveScenesMap();
}

void SceneManager::LoadScene(const DCore::UUIDType& uuid, sceneRefType* outScene)
{
	//  Its assumed that the user will never try to load the same scene at same time.
	const DCore::DString uuidString((std::string)uuid);
	DASSERT_E(s_scenesNode[uuidString.Data()]);
	DASSERT_E(s_scenesNode[uuidString.Data()][s_nameKey]);
	DASSERT_E(s_scenesNode[uuidString.Data()][s_pathKey]);
	const DCore::DString sceneName(s_scenesNode[uuidString.Data()][s_nameKey].as<std::string>());
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsSceneLoaded(uuid))
		{
			Log::Get().TerminalLog("Scene \"", sceneName.Data(), "\" is already loaded.");
			Log::Get().ConsoleLog(LogLevel::Error, "Scene \"%s\" is already loaded.", sceneName.Data());
			return;
		}
	}
	DCore::Scene scene(sceneName);
	DCore::SceneRef sceneRef(DCore::AssetManager::Get().LoadScene(uuid, std::move(scene)));
	const std::filesystem::path scenePath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_scenesNode[uuidString.Data()][s_pathKey].as<std::string>());
	DCore::ReturnError error(SceneSerialization::Get().DeserializeScene(scenePath, sceneRef));
	if (!error.Ok)
	{
		Log::Get().TerminalLog(error.Message);
		Log::Get().ConsoleLog(LogLevel::Error, "%s", error.Message.Data());
		DASSERT_E(false);
	}
	if (outScene != nullptr)
	{
		*outScene = sceneRef;
	}
}

void SceneManager::LoadSceneWithName(const stringType& sceneName, sceneRefType* outSceneRef)
{
	for (YAML::const_iterator it(s_scenesNode.begin()); it != s_scenesNode.end(); it++)
	{
		YAML::Node sceneInfoNode(it->second);
		DASSERT_E(sceneInfoNode[s_nameKey]);
		if (sceneInfoNode[s_nameKey].as<stringType>() != sceneName)
		{
			continue;
		}
		const DCore::UUIDType uuid(it->first.as<stringType>());
		LoadScene(uuid, outSceneRef);
	}
}

void SceneManager::GetSceneName(const uuidType& sceneUUID, stringType& outSceneName)
{
	const stringType uuidString((stringType)sceneUUID);
	DASSERT_E(s_scenesNode[uuidString]);
	DASSERT_E(s_scenesNode[uuidString][s_nameKey]);
	outSceneName = s_scenesNode[uuidString][s_nameKey].as<stringType>();
}

void SceneManager::SaveLoadedScenes()
{
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnLoadedScenes
	(
		[&](DCore::SceneRef sceneRef) -> bool
		{
			DCore::UUIDType uuid;
			sceneRef.GetUUID(uuid);
			const DCore::DString uuidString(((std::string)uuid).c_str());
			DASSERT_E(s_scenesNode[uuidString.Data()]);
			DASSERT_E(s_scenesNode[uuidString.Data()][s_pathKey]);
			std::filesystem::path scenePath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_scenesNode[uuidString.Data()][s_pathKey].as<std::string>());
			SceneSerialization::Get().SerializeScene(scenePath, sceneRef);
			return false;
		}
	);
}

void SceneManager::DeleteScene(const uuidType& uuid)
{
	// Delete the scene file from scene directory.
	const std::string uuidString(uuid);
	DASSERT_E(s_scenesNode[uuidString][s_pathKey]);
	const std::filesystem::path scenePath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_scenesNode[uuidString][s_pathKey].as<std::string>());
	std::filesystem::remove(scenePath);
	// Remove its key from s_scenesNode.
	YAML::Node newScenesNode;
	for (YAML::const_iterator it(s_scenesNode.begin()); it != s_scenesNode.end(); it++)
	{
		const uuidType sceneUUID(it->first.as<std::string>());
		if (sceneUUID == uuid)
		{
			continue;
		}
		newScenesNode[it->first] = it->second;
	}
	s_scenesNode = newScenesNode;
	// Save the new s_scenesNode.
	YAML::Emitter emitter;
	emitter << s_scenesNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(GetSceneDirectoryPath() / s_scenesFileName);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	// Unload the scene from SceneAssetManager.
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	if (DCore::AssetManager::Get().IsSceneLoaded(uuid))
	{
		DCore::AssetManager::Get().UnloadScene(uuid);
	}
}

bool SceneManager::RenameScene(const uuidType& uuid, const stringType& newName)
{
	for (YAML::const_iterator it(s_scenesNode.begin()); it != s_scenesNode.end(); it++)
	{
		if (newName == it->second[s_nameKey].as<stringType>())
		{
			Log::Get().TerminalLog("Scene with name ", newName.c_str(), " is already created.");
			Log::Get().ConsoleLog(LogLevel::Error, "Scene with name %s is already created.", newName.c_str());
			return false;
		}
	}
	const stringType uuidString(uuid);
	DASSERT_E(s_scenesNode[uuidString][s_pathKey]);
	const std::filesystem::path oldPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_scenesNode[uuidString][s_pathKey].as<stringType>());
	std::filesystem::path newPath(oldPath);
	newPath.replace_filename(newName + s_sceneFileExtension);
	YAML::Node sceneNode(YAML::LoadFile(oldPath.string()));
	DASSERT_E(sceneNode[s_sceneKey][s_nameKey]);
	sceneNode[s_sceneKey][s_nameKey] = newName;
	YAML::Emitter emitter;
	emitter << sceneNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(oldPath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	std::filesystem::rename(oldPath, newPath);
	DASSERT_E(s_scenesNode[uuidString][s_nameKey]);
	s_scenesNode[uuidString][s_nameKey] = newName;
	s_scenesNode[uuidString][s_pathKey] = Path::Get().MakePathRelativeToAssetsDirectory(newPath).string().c_str();
	SaveScenesMap();
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	if (DCore::AssetManager::Get().IsSceneLoaded(uuid))
	{
		DCore::AssetManager::Get().RenameScene(uuid, newName);
	}
	return true;
}

std::filesystem::path SceneManager::GetSceneDirectoryPath() const
{
	return std::filesystem::path(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_scenesDirectory);
}
 
void SceneManager::GenerateSceneThumbnail(const stringType& uuidString, const pathType& thumbnailPath) const
{
	std::ofstream ostream(thumbnailPath);	
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	emitter << YAML::Key << s_uuidKey << YAML::Value << uuidString;
	emitter << YAML::EndMap;
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

void SceneManager::SaveScenesMap() const
{
	std::ofstream ostream(GetSceneDirectoryPath() / s_scenesFileName);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << s_scenesNode;
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

}
