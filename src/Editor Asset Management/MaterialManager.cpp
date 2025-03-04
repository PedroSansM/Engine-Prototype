#include "MaterialManager.h"
#include "ConsolePanel.h"
#include "Log.h"
#include "ProgramContext.h"
#include "Path.h"
#include "TextureManager.h"
#include "SceneManager.h"

#include "yaml-cpp/yaml.h"

#include <cstdint>
#include <fstream>
#include <iterator>



namespace DEditor
{

static YAML::Node s_materialsNode;							// Maps UUID -> Material path.
static const char* s_uuidKey = "UUID";
static const char* s_typeKey = "Type";
static const char* s_nameKey = "Name";
static const char* s_ambientMapKey = "Ambient Map Tex";
static const char* s_diffuseMapKey = "Diffuse Map Tex";
static const char* s_specularMapKey = "Specular Map Tex";
static const char* s_glossinessKey = "Glossiness";
static const char* s_diffuseColorKey = "Diffuse Color";
static const char* s_materialsFileName = "materials.dmaterials";
static const char* s_materialDirectory = "material";

MaterialManager::MaterialManager()
{
	std::filesystem::path materialsPath(GetMaterialsPath() / s_materialsFileName);
	std::ofstream ostream(materialsPath, std::ios_base::out | std::ios_base::app);
	DASSERT_K(ostream);
	ostream.close();
	DASSERT_K(ostream);
	s_materialsNode = YAML::LoadFile(materialsPath.string());
}

void MaterialManager::CreateSpriteMaterial(spriteMaterialType type, const stringType& materialName, const pathType& thumbnailDirectory)
{
	for (const auto& dirEntry : std::filesystem::directory_iterator(GetMaterialsPath()))
	{
		if (materialName == dirEntry.path().stem())
		{
			Log::Get().TerminalLog("Material with name ", materialName.c_str(), " is already created.");
			Log::Get().ConsoleLog(LogLevel::Error, "Material with name %s is already created.", materialName.c_str());
			return;
		}
	}
	pathType materialPath(GetMaterialsPath() / materialName);
	materialPath += ".dsprmat";
	std::ifstream istream(materialPath);
	if (istream)
	{
		Log::Get().TerminalLog("Sprite material with name ", materialName.c_str(), " is already created.");
		Log::Get().ConsoleLog(LogLevel::Error, "Sprite material with name %s is already created.", materialName.c_str());
		istream.close();
		return;
	}
	std::ofstream ostream(materialPath);
	DASSERT_K(ostream);
	uuidType uuid;
	DCore::UUIDGenerator::Get().GenerateUUID(uuid);
	const stringType uuidString(((stringType)uuid).c_str());
	const stringType spriteMaterialTypeString(DCore::SpriteMaterial::SpriteMaterialTypeToString(type));
	s_materialsNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(materialPath).string().c_str();
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	emitter << YAML::Key << s_typeKey << YAML::Value << spriteMaterialTypeString;
	emitter << YAML::Key << s_nameKey << YAML::Value << materialName.c_str();
	emitter << YAML::Key << s_ambientMapKey << YAML::Value << "";
	emitter << YAML::Key << s_diffuseMapKey << YAML::Value << "";
	emitter << YAML::Key << s_specularMapKey << YAML::Value << "";
	emitter << YAML::Key << s_glossinessKey << YAML::Value << 0.5f;
	emitter << YAML::Key << s_diffuseColorKey << YAML::Value << YAML::Flow << YAML::BeginSeq << 1.0f << 1.0f << 1.0f << 1.0f << YAML::EndSeq;
	emitter << YAML::EndMap;
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	pathType thumbailPath(thumbnailDirectory / materialName.c_str());
	thumbailPath += ".dtsprmat";
	GenerateSpriteMaterialThumbail(thumbailPath, uuidString, spriteMaterialTypeString, materialName);
	SaveMaterialsMap();
}

DCore::SpriteMaterialRef MaterialManager::LoadSpriteMaterial(const DCore::UUIDType& uuid)
{
	{
		DCore::ReadWriteLockGuard materialGuard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsSpriteMaterialLoaded(uuid))
		{
			DCore::ReadWriteLockGuard textureGuard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
			return DCore::AssetManager::Get().GetSpriteMaterial(uuid);
		}
	}
	const stringType uuidString(((stringType)uuid).c_str());
	DASSERT_E(s_materialsNode[uuidString.c_str()]);	
	pathType materialPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_materialsNode[uuidString.c_str()].as<stringType>());
	YAML::Node materialNode(YAML::LoadFile(materialPath.string()));
	DASSERT_K(materialNode[s_typeKey]);
	DASSERT_K(materialNode[s_nameKey]);
	DASSERT_K(materialNode[s_ambientMapKey]);
	DASSERT_K(materialNode[s_diffuseMapKey]);
	DASSERT_K(materialNode[s_specularMapKey]);
	DASSERT_K(materialNode[s_glossinessKey]);
	DASSERT_K(materialNode[s_diffuseColorKey]);
	const stringType typeString(materialNode[s_typeKey].as<stringType>().c_str());
	const stringType ambientMapString(materialNode[s_ambientMapKey].as<stringType>().c_str());
	const stringType diffuseMapString(materialNode[s_diffuseMapKey].as<stringType>().c_str());
	const stringType specularMapString(materialNode[s_specularMapKey].as<stringType>().c_str());
	DCore::DFloat glossiness(materialNode[s_glossinessKey].as<float>());
	YAML::Node diffuseColorNode(materialNode[s_diffuseColorKey]);
	DCore::DVec4 diffuseColor(diffuseColorNode[0].as<float>(), diffuseColorNode[1].as<float>(), diffuseColorNode[2].as<float>(), diffuseColorNode[3].as<float>());
	bool ambientMapDefined(ambientMapString != "");
	bool diffuseMapDefined(diffuseMapString != "");
	bool specularMapDefined(specularMapString != "");
	DCore::SpriteMaterialType type(DCore::SpriteMaterial::StringToSpriteMaterialType(typeString));
	DCore::SpriteMaterial spriteMaterial(type);
	spriteMaterial.SetGlossiness(glossiness);
	spriteMaterial.SetDiffuseColor(diffuseColor);
	spriteMaterial.SetName(materialNode[s_nameKey].as<std::string>());
	if (ambientMapDefined)
	{
		uuidType uuid(ambientMapString.c_str());
		DCore::Texture2DRef ambientMapRef(TextureManager::Get().LoadTexture2D(uuid));
		spriteMaterial.SetAmbientMapRef(ambientMapRef);
	}
	if (diffuseMapDefined)
	{
		uuidType uuid(diffuseMapString.c_str());
		DCore::Texture2DRef diffuseMapRef(TextureManager::Get().LoadTexture2D(uuid));
		spriteMaterial.SetDiffuseMapRef(diffuseMapRef);
	}
	if (specularMapDefined)
	{
		uuidType uuid(specularMapString.c_str());
		DCore::Texture2DRef specularMapRef(TextureManager::Get().LoadTexture2D(uuid));
		spriteMaterial.SetSpecularMapRef(specularMapRef);
	}
	return DCore::AssetManager::Get().LoadSpriteMaterial(uuid, std::move(spriteMaterial));
}

MaterialManager::pathType MaterialManager::GetMaterialsPath() const
{
	pathType materialsPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_materialDirectory);
	return materialsPath;
}

void MaterialManager::RemoveTextureReference(const uuidType& textureUUID)
{
	constexpr uint8_t numberOfTexKeys(3);
	static const char* texKeys[numberOfTexKeys]{s_ambientMapKey, s_diffuseMapKey, s_specularMapKey};
	for (YAML::const_iterator it(s_materialsNode.begin()); it != s_materialsNode.end(); it++)
	{
		const pathType materialPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / it->second.as<stringType>());
		YAML::Node materialNode(YAML::LoadFile(materialPath.string()));
		for (uint8_t i(0); i < numberOfTexKeys; i++)
		{
			const char* texKey(texKeys[i]);
			DASSERT_E(materialNode[texKey]);
			if (const stringType mapUUIDString(materialNode[texKey].as<stringType>()); !mapUUIDString.empty())
			{
				const uuidType mapUUID(mapUUIDString);
				if (mapUUID == textureUUID)
				{
					materialNode[texKey] = "";
				}
			}
		}
		YAML::Emitter emitter;
		emitter << materialNode;
		DASSERT_E(emitter.good());
		std::ofstream ostream(materialPath);
		DASSERT_E(ostream);
		ostream << emitter.c_str();
		ostream.close();
		DASSERT_E(ostream);
	}
}

void MaterialManager::SaveSpriteMaterial(const spriteMaterialRefType spriteMaterial)
{	
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
	const stringType uuidString(spriteMaterial.GetUUID());
	DASSERT_E(s_materialsNode[uuidString]);
	const pathType materialPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_materialsNode[uuidString].as<stringType>());
	YAML::Node materialNode(YAML::LoadFile(materialPath.string()));
	switch (spriteMaterial.GetType())
	{
	case DCore::SpriteMaterialType::Unlit:
	{
		DASSERT_E(materialNode[s_typeKey] &&
				materialNode[s_nameKey] &&
				materialNode[s_ambientMapKey] &&
				materialNode[s_diffuseMapKey] &&
				materialNode[s_specularMapKey] &&
				materialNode[s_glossinessKey] &&
				materialNode[s_diffuseColorKey]
		);
		if (spriteMaterial.GetDiffuseMapRef().IsValid())
		{
			materialNode[s_diffuseMapKey] = static_cast<stringType>(spriteMaterial.GetDiffuseMapRef().GetUUID());
		}
		else
		{
			materialNode[s_diffuseMapKey] = "";
		}
		char buff[128];
		const DCore::DVec4 diffuseColor(spriteMaterial.GetDiffuseColor());
		std::sprintf(buff, "[%f, %f, %f, %f]", diffuseColor.x, diffuseColor.y, diffuseColor.z, diffuseColor.w);
		materialNode[s_diffuseColorKey] = YAML::Load(buff);
		YAML::Emitter emitter;
		emitter << materialNode;
		DASSERT_E(emitter.good());
		std::ofstream ostream(materialPath);
		DASSERT_E(ostream);
		ostream << emitter.c_str();
		ostream.close();
		DASSERT_E(ostream);
		return;
	}	
	case DCore::SpriteMaterialType::Lit:
		return;
	default:
		return;
	}
}

void MaterialManager::DeleteSpriteMaterial(const uuidType& materialUUID)
{
	const stringType uuidString(materialUUID);
	DASSERT_E(s_materialsNode[uuidString]);
	const pathType materialPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_materialsNode[uuidString].as<stringType>());
	std::filesystem::remove(materialPath);
	YAML::Node newMaterialsNode;
	for (YAML::const_iterator it(s_materialsNode.begin()); it != s_materialsNode.end(); it++)
	{
		const uuidType uuid(it->first.as<stringType>());
		if (uuid == materialUUID)
		{
			continue;
		}
		newMaterialsNode[it->first] = it->second;
	}
	s_materialsNode = newMaterialsNode;
	YAML::Emitter emitter;
	emitter << s_materialsNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(GetMaterialsPath() / s_materialsFileName);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsSpriteMaterialLoaded(materialUUID))
		{
			DCore::AssetManager::Get().UnloadSpriteMaterial(materialUUID, true);
		}
	}
	SceneManager::Get().SaveLoadedScenes();
}

bool MaterialManager::RenameSpriteMaterial(const uuidType& uuid, const stringType& newName)
{
	for (YAML::const_iterator it(s_materialsNode.begin()); it != s_materialsNode.end(); it++)
	{
		YAML::Node materialNode(YAML::LoadFile((ProgramContext::Get().GetProjectAssetsDirectoryPath() / it->second.as<stringType>()).string()));
		DASSERT_E(materialNode[s_nameKey]);
		if (materialNode[s_nameKey].as<stringType>() == newName)
		{
			Log::Get().TerminalLog("There is already a sprite material with name %s", newName.c_str());
			Log::Get().ConsoleLog(LogLevel::Error, "There is already a sprite material with name %s", newName.c_str());
			return false;
		}
	}
	const stringType uuidString(uuid);
	DASSERT_E(s_materialsNode[uuidString]);
	const pathType oldPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_materialsNode[uuidString].as<stringType>());
	YAML::Node materialNode(YAML::LoadFile(oldPath.string()));
	DASSERT_E(materialNode[s_nameKey]);
	materialNode[s_nameKey] = newName;
	YAML::Emitter emitter;
	emitter << materialNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(oldPath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	pathType newPath(oldPath);
	newPath.replace_filename(newName + ".dsprmat");
	std::filesystem::rename(oldPath, newPath);
	s_materialsNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(newPath).string().c_str();
	SaveMaterialsMap();
	return true;
}

bool MaterialManager::SpriteMaterialExists(const uuidType& uuid)
{
	for (YAML::const_iterator it(s_materialsNode.begin()); it != s_materialsNode.end(); it++)	
	{
		const uuidType materialUUID(it->first.as<stringType>());
		if (materialUUID == uuid)
		{
			return true;
		}
	}
	return false;
}

void MaterialManager::GenerateSpriteMaterialThumbail(const pathType& thumbailPath, const stringType& uuidString, const stringType& spriteMaterialTypeString, const stringType& materialName)
{
	std::ofstream ostream(thumbailPath);
	DASSERT_K(ostream);
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	emitter << YAML::Key << "UUID" << YAML::Value << uuidString.c_str();
	emitter << YAML::EndMap;
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_K(ostream);
}

void MaterialManager::SaveMaterialsMap()
{
	std::filesystem::path materialsFilePath(GetMaterialsPath() / s_materialsFileName);
	std::ofstream ostream(materialsFilePath);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << s_materialsNode;
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

}
