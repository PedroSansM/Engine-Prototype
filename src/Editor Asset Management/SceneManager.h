#pragma once

#include "DommusCore.h"

#include <filesystem>
#include <string>



namespace DEditor
{

class SceneManager
{
public:
	using uuidType = DCore::UUIDType;
	using sceneRefType = DCore::SceneRef;
	using stringType = std::string;
	using pathType = std::filesystem::path;
public:
	~SceneManager() = default;
public:
	static SceneManager& Get()
	{
		static SceneManager sceneManager;
		return sceneManager;
	}
public:
	void CreateScene(const stringType& sceneName, const pathType& thumbnailDirectory);
	void LoadScene(const uuidType&, sceneRefType* outSceneRef = nullptr);
	void LoadSceneWithName(const stringType&, sceneRefType* outSceneRef = nullptr);
	void GetSceneName(const uuidType&, stringType& outSceneName);
	void SaveLoadedScenes();
	void DeleteScene(const uuidType&);
	bool RenameScene(const uuidType&, const stringType& newName);
private:
	SceneManager();
private:
	pathType GetSceneDirectoryPath() const;
	void GenerateSceneThumbnail(const stringType& uuidString, const pathType& thumbnailPath) const;
	void SaveScenesMap() const;
};

}
