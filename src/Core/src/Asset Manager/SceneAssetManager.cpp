#include "SceneAssetManager.h"



namespace DCore
{

SceneAssetManager::SceneAssetManager()
	:
	m_scenes(maximumNumberOfLoadedScenes)
{}

SceneAssetManager::~SceneAssetManager()
{
	m_scenes.Clear();
	m_loadedScenes.clear();
}

bool SceneAssetManager::IsSceneLoaded(const UUIDType& uuid)
{
	return m_loadedScenes.find(uuid) != m_loadedScenes.end();
}

SceneRef SceneAssetManager::LoadScene(const UUIDType& uuid, Scene&& scene)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	DASSERT_E(m_scenes.Size() <= maximumNumberOfLoadedScenes);
	DASSERT_E(m_loadedScenes.find(uuid) == m_loadedScenes.end());
	InternalSceneRefType internalRef(m_scenes.PushBack(uuid, std::move(scene)));
	m_loadedScenes.insert({uuid, internalRef});
	return SceneRef(internalRef, m_lockData);
}

void SceneAssetManager::UnloadScene(const UUIDType& uuid)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	UnloadSceneNoBlock(uuid);
}

SceneRef SceneAssetManager::GetSceneRefFromSceneRefId(SceneIdType sceneId)
{
	return SceneRef(m_scenes.GetRefFromRefId(sceneId), m_lockData);
}

void SceneAssetManager::ClearScenes()
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	m_scenes.Clear();
	m_loadedScenes.clear();
}

void SceneAssetManager::GetScenesInfo(sceneContainerType& outScenes, std::unordered_map<UUIDType, InternalSceneRefType>& outLoadedScenes)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	outScenes = std::move(m_scenes);
	outLoadedScenes = std::move(m_loadedScenes);
	m_scenes.Clear();
	m_scenes.Reserve(maximumNumberOfLoadedScenes);
	m_loadedScenes.clear();
}

void SceneAssetManager::SetScenesInfo(sceneContainerType&& scenes, std::unordered_map<UUIDType, InternalSceneRefType>&& loadedScenes)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	std::vector<UUIDType> scenesToUnload;
	scenesToUnload.reserve(loadedScenes.size());
	for (auto& pair : m_loadedScenes)
	{
		if (loadedScenes.find(pair.first) == loadedScenes.end())
		{
			scenesToUnload.push_back(pair.first);
		}
	}
	for (const UUIDType& uuid : scenesToUnload)
	{
		UnloadSceneNoBlock(uuid);
	}
	m_scenes = std::move(scenes);
	m_loadedScenes = std::move(loadedScenes);
}

void SceneAssetManager::RenameScene(const UUIDType& uuid, const stringType& name)
{
	ReadWriteLockGuard guard(LockType::WriteLock, m_lockData);
	auto it(m_loadedScenes.find(uuid));
	DASSERT_E(it != m_loadedScenes.end());
	it->second->GetAsset().SetName(name);
}

void SceneAssetManager::UnloadSceneNoBlock(const UUIDType& uuid)
{
	auto it(m_loadedScenes.find(uuid));
	if (it == m_loadedScenes.end())
	{
		return;
	}
	if (it->second->GetReferenceCount() == 1)
	{
		m_scenes.Remove(it->second);
		m_loadedScenes.erase(uuid);
		return;
	}
	it->second->SubReferenceCount();
}

}
