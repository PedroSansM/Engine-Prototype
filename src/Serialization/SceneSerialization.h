#pragma once

#include "DommusCore.h"

#include <filesystem>



namespace DEditor
{

class SceneSerialization
{
public:
	using returnErrorType = DCore::ReturnError;
	using uuidType = DCore::UUIDType;
	using stringType = std::string;
	using pathType = std::filesystem::path;
	using sceneRefType = DCore::SceneRef;
public:
	~SceneSerialization() = default;
public:
	static SceneSerialization& Get()
	{
		static SceneSerialization sceneSerialilzation;
		return sceneSerialilzation;
	}
public:
	returnErrorType DeserializeScene(const pathType& scenePath, sceneRefType);

	// Requires SceneAssetManager lock.
	returnErrorType SerializeScene(const pathType& scenePath, sceneRefType);
private:
	SceneSerialization() = default;
};

}
