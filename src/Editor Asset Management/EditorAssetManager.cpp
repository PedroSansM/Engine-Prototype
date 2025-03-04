#include "EditorAssetManager.h"
#include "ProgramContext.h"
#include "Log.h"
#include "ProgramContext.h"
#include "SceneSerialization.h"
#include "TextureMetadataManager.h"
#include "TextureManager.h"

#include "yaml-cpp/yaml.h"

#include <ios>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>



namespace DEditor
{

void EditorAssetManager::ImportResource(const std::filesystem::path& path, const std::filesystem::path& targetDirectory)
{
	const DCore::DString extension(path.extension().string().c_str());
	if (extension == ".png")
	{
		TextureManager::Get().ImportTexture(path, targetDirectory);
		return;
	}
	Log::Get().TerminalLog("Fail to import resource at path: %s. Resource not supported.", path.string().c_str());
}

}
