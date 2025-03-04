#pragma once

#include "UUID.h"
#include "Scene.h"
#include "ReadWriteLockGuard.h"
#include "AssetManagerTypes.h"

#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <mutex>



namespace DCore
{


class SceneAssetManager
{
	friend class ReadWriteLockGuard;
public:
	using sceneContainerType = AssetContainerType<Scene, SceneIdType, SceneVersionType>;
	using loadedSceneContainerType = std::unordered_map<UUIDType, InternalSceneRefType>;
	using stringType = std::string;
public:
	static constexpr size_t maximumNumberOfLoadedScenes{64};
public:
	virtual ~SceneAssetManager();
public:
	bool IsSceneLoaded(const UUIDType&);
	[[nodiscard]] SceneRef LoadScene(const UUIDType&, Scene&&);
	void UnloadScene(const UUIDType&);
	SceneRef GetSceneRefFromSceneRefId(SceneIdType);	// Does not increase the reference count to the referenced scene.
	void ClearScenes();
	void GetScenesInfo(sceneContainerType& outScenes, std::unordered_map<UUIDType, InternalSceneRefType>& outLoadedScenes);
	void SetScenesInfo(sceneContainerType&&, std::unordered_map<UUIDType, InternalSceneRefType>&&);
	void RenameScene(const UUIDType& uuid, const stringType&);
public:
	template <class Func>
	void IterateOnLoadedScenes(Func function)
	{
		m_scenes.Iterate
		(
			[&](InternalSceneRefType internalScene) -> bool
			{
				return std::invoke(function, SceneRef(internalScene, m_lockData));
			}
		);
	}
protected:
	SceneAssetManager();
private:
	sceneContainerType m_scenes;
	loadedSceneContainerType m_loadedScenes;
	LockData m_lockData;
private:
	void UnloadSceneNoBlock(const UUIDType&);
private:
	LockData& GetLockData()
	{
		return m_lockData;
	}
};

}
