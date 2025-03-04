#pragma once

#include "Scene.h"

#include <functional>
#include <string>



namespace DCore
{

class SceneLoader
{
public:
	using stringType = std::string;
	using loadSceneFunctionType = std::function<SceneRef(const stringType&)>;
public:
	SceneLoader(const SceneLoader&) = delete;
	SceneLoader(SceneLoader&&) = delete;
	~SceneLoader() = default;
public:
	static SceneLoader& Get()
	{
		static SceneLoader instance;
		return instance;
	}
public:
	SceneRef LoadScene(const stringType& sceneName);
public:
	void SetLoadSceneFunc(loadSceneFunctionType&& loadSceneFunc)
	{
		m_loadSceneFunc = std::move(loadSceneFunc);
	}
private:
	SceneLoader() = default;
private:
	loadSceneFunctionType m_loadSceneFunc;
};

}