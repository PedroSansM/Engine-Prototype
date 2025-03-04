#pragma once

#include "SceneAssetManager.h"
#include "Texture2DAssetManager.h"
#include "SpriteMaterialAssetManager.h"
#include "AnimationAssetManager.h"
#include "AnimationStateMachineAssetManager.h"
#include "PhysicsMaterialAssetManager.h"



namespace DCore
{

class AssetManager
	:
	public SceneAssetManager,
	public Texture2DAssetManager,
	public SpriteMaterialAssetManager,
	public AnimationAssetManager,
	public AnimationStateMachineAssetManager,
	public PhysicsMaterialAssetManager
{
	friend class EntityQuery;
public:
	~AssetManager() = default;
public:
	static AssetManager& Get()
	{
		static AssetManager assetManager;
		return assetManager;
	}
private:
	AssetManager() = default;
};	

}
