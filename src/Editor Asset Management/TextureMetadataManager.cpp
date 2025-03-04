#include "TextureMetadataManager.h"
#include "Log.h"
#include "ProgramContext.h"
#include "Path.h"

#include "yaml-cpp/yaml.h"

#include <cassert>
#include <fstream>
#include <ios>
#include <string>



namespace DEditor
{

static YAML::Node s_texturesMetadataNode;

TextureMetadataManager::TextureMetadataManager()
{
	std::filesystem::path textureMetadatasPath(GetMetadatasPath());
	std::ofstream ostream(textureMetadatasPath, std::ios_base::app);
	assert(ostream);
	ostream.close();
	assert(ostream);
	s_texturesMetadataNode = YAML::LoadFile(textureMetadatasPath.string());
}
	 
void TextureMetadataManager::AddTextureMetadataFile(
	const std::filesystem::path& thumbnailDirectory, 
	const DCore::UUIDType& uuid,
	const std::filesystem::path& texturePath, 
	const DCore::DString& textureName)
{
	YAML::Node metadataNode;
	std::filesystem::path textureRelativePath(Path::Get().MakePathRelativeToAssetsDirectory(texturePath));
	metadataNode["SRGB"] = false;
	metadataNode["AlphaMask"] = false;
	metadataNode["Filter"] = "Bilinear";
	metadataNode["Path"] = textureRelativePath.string();
	s_texturesMetadataNode.force_insert((std::string)uuid, metadataNode);
	YAML::Emitter emitter;
	emitter << s_texturesMetadataNode;
	std::ofstream ostream(GetMetadatasPath());
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	GenerateTextureThumbnail(thumbnailDirectory, textureRelativePath, uuid, textureName);
}

DCore::Texture2DMetadata TextureMetadataManager::GetTextureMetadata(const DCore::UUIDType& uuid)
{
	DASSERT_E(s_texturesMetadataNode[(std::string)uuid]);
	YAML::Node textureMetadataNode(s_texturesMetadataNode[(std::string)uuid]);
	DASSERT_E(textureMetadataNode["SRGB"]);
	DASSERT_E(textureMetadataNode["AlphaMask"]);
	DASSERT_E(textureMetadataNode["Filter"]);
	return DCore::Texture2DMetadata 
	(
		textureMetadataNode["SRGB"].as<bool>(),
		textureMetadataNode["AlphaMask"].as<bool>(),
		DCore::Texture2DMetadata::StringToFilter(textureMetadataNode["Filter"].as<std::string>().c_str())
	);
}

std::filesystem::path TextureMetadataManager::GetMetadatasPath() const
{
	return std::filesystem::path(ProgramContext::Get().GetProjectAssetsDirectoryPath() / "texture" / "textureMetadatas.dtexMetadata");
}

void TextureMetadataManager::GenerateTextureThumbnail(
	const std::filesystem::path& thumbnailDirectory, 
	const std::filesystem::path& textureRelativePath, 
	const DCore::UUIDType& uuid, 
	const DCore::DString& textureName) const
{
	std::filesystem::path thumbnailPath(thumbnailDirectory / textureName.Data());
	thumbnailPath += ".dttex";
	YAML::Node thumbnailNode;
	thumbnailNode["UUID"] = (std::string)uuid;
	thumbnailNode["Path"] = textureRelativePath.string();
	YAML::Emitter emitter;
	emitter << thumbnailNode;
	std::ofstream ostream(thumbnailPath);
	assert(ostream);
	ostream << emitter.c_str();
	ostream.close();
	assert(ostream);
}

}
