#include "TextureManager.h"
#include "ConsolePanel.h"
#include "ProgramContext.h"
#include "Log.h"
#include "Path.h"
#include "MaterialManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "yaml-cpp/yaml.h"

#include <fstream>
#include <string>



namespace DEditor
{

static YAML::Node s_texturesNode;								// UUID -> metadata + in project path;
static const char* s_textureDirectoryName = "texture";
static const char* s_texturesFileName = "textures.dtex";
static const char* s_uuidKey = "UUID";
static const char* s_srgbKey = "SRGB";
static const char* s_alphaMaskKey = "Alpha Mask";
static const char* s_filterKey = "Filter";
static const char* s_pathKey = "Path";
static const char* s_thumbnailExtension = ".dttex";

TextureManager::TextureManager()
{
	std::filesystem::path texturesPath(GetTextureDirectoryPath() / s_texturesFileName);
	std::ofstream ostream(texturesPath, std::ios_base::out | std::ios_base::app);
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	s_texturesNode = YAML::LoadFile(texturesPath.string());
}

void TextureManager::ImportTexture(const std::filesystem::path& outsidePath, const std::filesystem::path& thumbnailDirectory)
{
	std::ifstream fromStream(outsidePath, std::ios_base::binary);
	if (!fromStream)
	{
		Log::Get().TerminalLog("Fail to get texture2D at path: %s", outsidePath.string().c_str());
		Log::Get().ConsoleLog(LogLevel::Error, "Fail to find texture2D at path: %s", outsidePath.string().c_str());
		return;
	}
	std::filesystem::path toPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_textureDirectoryName / outsidePath.filename());
	std::ofstream toStream(toPath, std::ios_base::binary);
	if (!toStream)
	{
		Log::Get().TerminalLog("Fail to copy texture2D at path: %s. to: %s", outsidePath.string().c_str(), toPath.string().c_str());
		fromStream.close();
		return;
	}
	toStream << fromStream.rdbuf();
	fromStream.close();
	toStream.close();
	DCore::UUIDType uuid;
	DCore::UUIDGenerator::Get().GenerateUUID(uuid);
	const DCore::DString uuidString(((std::string)uuid).c_str());
	YAML::Node textureMetadataNode;
	textureMetadataNode[s_srgbKey] = false;
	textureMetadataNode[s_alphaMaskKey] = false;
	textureMetadataNode[s_filterKey] = "Bilinear";
	textureMetadataNode[s_pathKey] = Path::Get().MakePathRelativeToAssetsDirectory(toPath).string().c_str();
	s_texturesNode[uuidString.Data()] = textureMetadataNode;
	std::filesystem::path thumbnailPath(thumbnailDirectory / toPath.filename().stem());
	thumbnailPath += s_thumbnailExtension;
	CreateTextureThumbnail(thumbnailPath, uuidString);
	SaveTexturesMap();
}

DCore::Texture2DRef TextureManager::LoadTexture2D(const DCore::UUIDType& uuid)
{
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsTexture2DLoaded(uuid))
		{
			return DCore::AssetManager::Get().GetTexture2DRef(uuid);
		}
	}
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, m_lockData);
		if (m_texturesLoading.count(uuid) == 0)
		{
			m_texturesLoading.insert(uuid);
			goto LoadTexture;
		}
	}
	while (true)
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsTexture2DLoaded(uuid))
		{
			return DCore::AssetManager::Get().GetTexture2DRef(uuid);
		}
	}
LoadTexture:
	const DCore::DString uuidString(((std::string)uuid).c_str());
	DASSERT_E(s_texturesNode[uuidString.Data()]);
	DASSERT_E(s_texturesNode[uuidString.Data()][s_pathKey]);
	std::filesystem::path texturePath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_texturesNode[uuidString.Data()][s_pathKey].as<std::string>());
	DCore::Texture2DMetadata metadata(GetTexture2DMetadata((std::string)uuid));
	int width(0), height(0), numberChannels(0);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* binary(stbi_load(texturePath.string().c_str(), &width, &height, &numberChannels, 0));
	DASSERT_E(binary != nullptr);
	DCore::Texture2DRef ref(DCore::AssetManager::Get().LoadTexture2D(uuid, binary, {width, height}, numberChannels, metadata));
	stbi_image_free(binary);
	DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, m_lockData);
	m_texturesLoading.erase(uuid);
	return ref;
}

DCore::Texture2D TextureManager::LoadRawTexture2D(const std::filesystem::path& path)
{
	int width(0), height(0), numberChannels(0);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* binary(stbi_load(path.string().c_str(), &width, &height, &numberChannels, 0));
	DASSERT_E(binary != nullptr && "Fail to load texture. Check program command line args.");
	DCore::DVec2 size(width, height);
	DCore::Texture2D texture2D(DCore::AssetManager::Get().LoadRawTexture2D(binary, size, numberChannels));
	stbi_image_free(binary);
	return texture2D;
}

TextureInfo TextureManager::GetTextureInfo(const pathType& path)
{
	int width(0), height(0), numberChannels(0);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* binary(stbi_load(path.string().c_str(), &width, &height, &numberChannels, 0));
	DASSERT_E(binary != nullptr && "Fail to load texture. Check program command line args.");
	DCore::DVec2 sizes(width, height);
	return {binary, sizes, numberChannels};
}

void TextureManager::DeleteTexture(const uuidType& uuid)
{
	const std::string uuidString(uuid);
	DASSERT_E(s_texturesNode[uuidString][s_pathKey]);
	const pathType texturePath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_texturesNode[uuidString][s_pathKey].as<std::string>());
	std::filesystem::remove(texturePath);
	YAML::Node newTexturesNode;
	for (YAML::const_iterator it(s_texturesNode.begin()); it != s_texturesNode.end(); it++)
	{
		const uuidType textureUUID(it->first.as<std::string>());
		if (textureUUID == uuid)
		{
			continue;
		}
		newTexturesNode[it->first] = it->second;
	}
	s_texturesNode = newTexturesNode;
	YAML::Emitter emitter;
	emitter << s_texturesNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(GetTextureDirectoryPath() / s_texturesFileName);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
	if (DCore::AssetManager::Get().IsTexture2DLoaded(uuid))
	{
		DCore::AssetManager::Get().UnloadTexture2D(uuid, true);
	}
	MaterialManager::Get().RemoveTextureReference(uuid);
}

bool TextureManager::RenameTexture(const uuidType& uuid, const stringType& newName)
{
	const stringType uuidString(uuid);
	for (YAML::const_iterator it(s_texturesNode.begin()); it != s_texturesNode.end(); it++)
	{
		YAML::Node textureInfoNode(it->second);
		DASSERT_E(textureInfoNode[s_pathKey]);
		const pathType texturePath(textureInfoNode[s_pathKey].as<stringType>());
		if (texturePath.stem() == newName)
		{
			Log::Get().TerminalLog("There is already a texture with name %s.", newName.c_str());
			Log::Get().ConsoleLog(LogLevel::Error, "There is already a texture with name %s.", newName.c_str());
			return false;
		}
	}
	DASSERT_E(s_texturesNode[uuidString][s_pathKey]);
	const pathType oldPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_texturesNode[uuidString][s_pathKey].as<stringType>());
	pathType newPath(oldPath);
	newPath.replace_filename(newName + ".png");
	std::filesystem::rename(oldPath, newPath);
	s_texturesNode[uuidString][s_pathKey] = Path::Get().MakePathRelativeToAssetsDirectory(newPath).string();
	SaveTexturesMap();
	return true;
}

void TextureManager::SaveTexture(textureRefType texture)
{
	const stringType uuidString(texture.GetUUID());
	DASSERT_E(s_texturesNode[uuidString][s_filterKey]);
	s_texturesNode[uuidString][s_filterKey] = DCore::Texture2DMetadata::FilterToString(texture.GetFilter());
	SaveTexturesMap();
}

void TextureManager::IterateOnTextures(textureIterationCallbackType callback)
{
	for (YAML::const_iterator it(s_texturesNode.begin()); it != s_texturesNode.end(); it++)
	{
		const uuidType uuid(it->first.as<stringType>());
		const pathType path(it->second[s_pathKey].as<stringType>());
		if (callback(uuid, path.stem().string()))
		{
			return;
		}
	}
}

bool TextureManager::TextureExists(const uuidType& uuid)
{
	const stringType uuidString(uuid);
	for (YAML::const_iterator it(s_texturesNode.begin()); it != s_texturesNode.end(); it++)
	{
		const uuidType textureUUID(it->first.as<stringType>());
		const stringType textureString(textureUUID);
		if (textureUUID == uuid)
		{
			return true;
		}
	}
	return false;
}

TextureManager::pathType TextureManager::GetTextureDirectoryPath() const
{
	return ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_textureDirectoryName;
}

void TextureManager::SaveTexturesMap() const
{
	std::ofstream ostream(GetTextureDirectoryPath() / s_texturesFileName);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << s_texturesNode;
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

void TextureManager::CreateTextureThumbnail(const std::filesystem::path& thumbnailPath, const DCore::DString& uuidString) const
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

DCore::Texture2DMetadata TextureManager::GetTexture2DMetadata(const DCore::DString& uuidString)
{
	YAML::Node metadataNode(s_texturesNode[uuidString.Data()]);
	DASSERT_E(metadataNode[s_srgbKey]);
	DASSERT_E(metadataNode[s_alphaMaskKey]);
	DASSERT_E(metadataNode[s_filterKey]);
	return DCore::Texture2DMetadata
	(
		metadataNode[s_srgbKey].as<bool>(),
		metadataNode[s_alphaMaskKey].as<bool>(),
		DCore::Texture2DMetadata::StringToFilter(metadataNode[s_filterKey].as<std::string>().c_str())
	);
}
 
}
