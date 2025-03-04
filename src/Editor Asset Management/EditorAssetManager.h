#pragma once

#include "DommusCore.h"

#include <filesystem>
#include <vector>



namespace DEditor
{

class EditorAssetManager
{
public:
	~EditorAssetManager() = default;
public:
	static EditorAssetManager& Get()
	{
		static EditorAssetManager assetManager;
		return assetManager;
	}
public:
	void ImportResource(const std::filesystem::path& path, const std::filesystem::path& targetDirectory);
private:
	EditorAssetManager() = default;
};

}
