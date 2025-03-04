#include "SpriteSheetGenManager.h"
#include "Path.h"
#include "ProgramContext.h"
#include "Log.h"
#include "MaterialManager.h"
#include "TextureManager.h"

#include "yaml-cpp/yaml.h"

#include <fstream>



namespace DEditor
{

static YAML::Node s_spriteSheetGensNode;
static const char* s_spriteSheetGensFileName("spriteSheetGens.dssgens");
static const char* s_nameKey("Name");
static const char* s_numberOfColumnsKey("Number of Columns");
static const char* s_texturesKey("Textures");
static const char* s_uuidKey("UUID");
static const char* s_texturesAlignmentKey("Alignment");

const char* SpriteSheetGenManager::spriteSheetGenFileExtension(".dssgen");
const char* SpriteSheetGenManager::spriteSheetGenThumbnailExtension(".dtssgen");
const char* SpriteSheetGenManager::spriteSheetGenDirectoryName("sprite sheet gen");

SpriteSheetGenManager::SpriteSheetGenManager()
	:
	m_spriteSheetGens(maxNumberOfSpriteSheetGensOpened)
{
	const pathType spriteSheetGensPath(GetSpriteSheetGenDirectory() / s_spriteSheetGensFileName);
	std::ofstream ostream(spriteSheetGensPath, std::ios_base::out | std::ios_base::app);
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	s_spriteSheetGensNode = YAML::LoadFile(spriteSheetGensPath.string());
}

SpriteSheetGenManager::~SpriteSheetGenManager()
{
	m_spriteSheetGens.Clear();
}

void SpriteSheetGenManager::CreateSpriteSheetGen(const stringType& name, const pathType& thumbnailDirectory)
{
	for (YAML::const_iterator it(s_spriteSheetGensNode.begin()); it != s_spriteSheetGensNode.end(); it++)
	{
		if (name == it->second.as<stringType>())
		{
			Log::Get().TerminalLog("Sprite sheet generator with name ", name.c_str(), " is already created.");
			Log::Get().ConsoleLog(LogLevel::Error, "Sprite sheet generator with name %s is already created.", name.c_str());
			return;
		}
	}
	DCore::UUIDType uuid;
	DCore::UUIDGenerator::Get().GenerateUUID(uuid);
	const stringType uuidString(uuid);
	const pathType spriteSheetGenFileName(name + spriteSheetGenFileExtension);
	const pathType spriteSheetGenFullPath(GetSpriteSheetGenDirectory() / spriteSheetGenFileName);
	YAML::Node spriteSheetGenNode;
	spriteSheetGenNode[s_nameKey] = name;
	spriteSheetGenNode[s_numberOfColumnsKey] = 1;
	spriteSheetGenNode[s_texturesKey] = YAML::Load("[]");
	YAML::Emitter emitter;
	emitter << spriteSheetGenNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(spriteSheetGenFullPath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	s_spriteSheetGensNode[uuidString] = (pathType(spriteSheetGenDirectoryName) / spriteSheetGenFileName).string();
	GenerateSpriteSheetGenThumbnail(uuidString, thumbnailDirectory / (name + spriteSheetGenThumbnailExtension));
	SaveSpriteSheetGensMap();
}

bool SpriteSheetGenManager::IsSpriteSheetGenLoaded(const uuidType& uuid)
{
	bool returnValue(false);
	m_spriteSheetGens.Iterate
	(
		[&](spriteSheetGenRefType spriteSheetGen) -> bool
		{
			if (spriteSheetGen->GetUUID() == uuid)
			{
				returnValue = true;
				return true;
			}
			return false;
		}
	);
	return returnValue;
}

SpriteSheetGenManager::spriteSheetGenRefType SpriteSheetGenManager::LoadSpriteSheetGen(const uuidType& uuid)
{
	DASSERT_E(!IsSpriteSheetGenLoaded(uuid));
	if (m_spriteSheetGens.Size() == maxNumberOfSpriteSheetGensOpened)
	{
		Log::Get().TerminalLog("%s", "Cannot open any more sprite sheet generators");
		Log::Get().ConsoleLog(LogLevel::Warning, "%s", "Cannot open any more sprite sheet generators");
		return spriteSheetGenRefType();
	}
	const stringType uuidString(uuid);
	DASSERT_E(s_spriteSheetGensNode[uuidString]);
	const pathType spriteSheetGenFullPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_spriteSheetGensNode[uuidString].as<stringType>());
	const YAML::Node spriteSheetGenNode(YAML::LoadFile(spriteSheetGenFullPath.string()));
	DASSERT_E(spriteSheetGenNode[s_nameKey] && spriteSheetGenNode[s_numberOfColumnsKey] && spriteSheetGenNode[s_texturesKey]);
	const stringType name(spriteSheetGenNode[s_nameKey].as<stringType>());
	spriteSheetGenRefType spriteSheetGen(m_spriteSheetGens.PushBack(uuid, name));
	size_t numberOfColumns(spriteSheetGenNode[s_numberOfColumnsKey].as<size_t>());
	spriteSheetGen->SetNumberOfColumns(numberOfColumns);
	const YAML::Node texturesNode(spriteSheetGenNode[s_texturesKey]);
	for (YAML::const_iterator it(texturesNode.begin()); it != texturesNode.end(); it++)
	{
		const YAML::Node spriteSheetElementNode(*it);
		DASSERT_E(spriteSheetElementNode[s_uuidKey] && spriteSheetElementNode[s_texturesAlignmentKey]);
		const uuidType uuid(spriteSheetElementNode[s_uuidKey].as<stringType>());
		if (!TextureManager::Get().TextureExists(uuid))
		{
			Log::Get().TerminalLog("%s", "Invalid texture reference in sprite sheet generator");
			Log::Get().ConsoleLog(LogLevel::Warning, "%s", "Invalid texture reference in sprite sheet generator");
			continue;
		}
		size_t elementIndex(spriteSheetGen->AddSprite(uuid));
		spriteSheetGen->UpdateSpriteOffset(elementIndex, {spriteSheetElementNode[s_texturesAlignmentKey][0].as<float>(), spriteSheetElementNode[s_texturesAlignmentKey][1].as<float>()});
	}
	return spriteSheetGen;
}

bool SpriteSheetGenManager::RenameSpriteSheetGen(const uuidType& uuid, const stringType& newName)
{
	for (YAML::const_iterator it(s_spriteSheetGensNode.begin()); it != s_spriteSheetGensNode.end(); it++)
	{
		const pathType path(it->second.as<stringType>());
		if (path.stem() == newName)
		{
			Log::Get().TerminalLog("There is already a sprite sheet generator with name %s.", newName.c_str());
			Log::Get().ConsoleLog(LogLevel::Error, "There is already a sprite sheet generator with name %s.", newName.c_str());
			return false;
		}
	}
	const stringType uuidString(uuid);
	DASSERT_E(s_spriteSheetGensNode[uuidString]);
	const pathType spriteSheetGenFullPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_spriteSheetGensNode[uuidString].as<stringType>());
	const pathType newPath(GetSpriteSheetGenDirectory() / (newName + spriteSheetGenFileExtension));
	s_spriteSheetGensNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(newPath).string();
	std::filesystem::rename(spriteSheetGenFullPath, newPath);
	m_spriteSheetGens.Iterate
	(
		[&](spriteSheetGenRefType spriteSheetGen) -> bool
		{
			if (spriteSheetGen->GetUUID() == uuid)
			{
				spriteSheetGen->SetName(newName);
			}
			return false;
		}
	);
	YAML::Node spriteSheetGenNode(YAML::LoadFile(newPath.string()));
	DASSERT_E(spriteSheetGenNode[s_nameKey]);
	spriteSheetGenNode[s_nameKey] = newName;
	YAML::Emitter emitter;
	emitter << spriteSheetGenNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(newPath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	SaveSpriteSheetGensMap();
	return true;
}

void SpriteSheetGenManager::UnloadSpriteSheetGen(spriteSheetGenRefType spriteSheetGen)
{
	if (!spriteSheetGen.IsValid())
	{
		return;
	}
	if (!IsSpriteSheetGenLoaded(spriteSheetGen->GetUUID()))
	{
		return;
	}
	m_spriteSheetGens.Remove(spriteSheetGen);
}

void SpriteSheetGenManager::DeleteSpriteSheetGen(const uuidType& uuid)
{
	const stringType uuidString(uuid);
	DASSERT_E(s_spriteSheetGensNode[uuidString]);
	const pathType spriteSheetGenFullPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_spriteSheetGensNode[uuidString].as<stringType>());
	std::filesystem::remove(spriteSheetGenFullPath);
	YAML::Node newSpriteSheetGensNode;
	for (YAML::const_iterator it(s_spriteSheetGensNode.begin()); it != s_spriteSheetGensNode.end(); it++)
	{
		if (static_cast<uuidType>(it->first.as<stringType>()) != uuid)
		{
			newSpriteSheetGensNode[it->first] = it->second;
		}
	}
	s_spriteSheetGensNode = newSpriteSheetGensNode;
	SaveSpriteSheetGensMap();
	m_spriteSheetGens.Iterate
	(
		[&](spriteSheetGenRefType spriteSheetGen) -> bool
		{
			if (spriteSheetGen->GetUUID() == uuid)
			{
				m_spriteSheetGens.Remove(spriteSheetGen);
				return true;
			}
			return false;
		}
	);
}

void SpriteSheetGenManager::SaveSpriteSheetGen(spriteSheetGenRefType spriteSheetGen)
{
	const stringType& uuidString(spriteSheetGen->GetUUID());
	DASSERT_E(s_spriteSheetGensNode[uuidString]);
	const pathType& spriteSheetGenFullPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_spriteSheetGensNode[uuidString].as<stringType>());
	const DCore::Registry& registry(spriteSheetGen->GetRegistry());
	DCore::ReadWriteLockGuard materialGuard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
	DCore::ReadWriteLockGuard textureGuard(DCore::LockType::ReadLock, *static_cast<DCore::Texture2DAssetManager*>(&DCore::AssetManager::Get()));
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	{
		emitter << YAML::Key << s_nameKey << YAML::Value << spriteSheetGen->GetName();
		emitter << YAML::Key << s_numberOfColumnsKey << YAML::Value << spriteSheetGen->GetNumberOfColumns();
		emitter << YAML::Key << s_texturesKey << YAML::Value << YAML::BeginSeq;
		{
			spriteSheetGen->IterateOnElements
			(
				[&](SpriteSheetElement& element) -> bool
				{
					auto [transformComponent, spriteComponent] = registry.GetComponents<DCore::TransformComponent, DCore::SpriteComponent>(element.Entity);
					if (!spriteComponent->GetSpriteMaterial().GetDiffuseMapRef().IsValid() || !TextureManager::Get().TextureExists(spriteComponent->GetSpriteMaterial().GetDiffuseMapRef().GetUUID()))
					{
						return false;
					}
					emitter << YAML::BeginMap;
					{
						emitter << YAML::Key << s_uuidKey << YAML::Value << static_cast<stringType>(spriteComponent->GetSpriteMaterial().GetDiffuseMapRef().GetUUID());
						const DCore::DVec3& translation(transformComponent->GetTranslation());
						emitter << YAML::Key << s_texturesAlignmentKey << YAML::Flow << YAML::BeginSeq << static_cast<int>(translation.x) << static_cast<int>(translation.y) << YAML::EndSeq;
						emitter << YAML::EndMap;
					}
					return false;
				}
			);
			emitter << YAML::EndSeq;
		}
		emitter << YAML::EndMap;
	}
	DASSERT_E(emitter.good());
	std::ofstream ostream(spriteSheetGenFullPath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

SpriteSheetGenManager::pathType SpriteSheetGenManager::GetSpriteSheetGenDirectory() const
{
	return pathType(ProgramContext::Get().GetProjectAssetsDirectoryPath() / spriteSheetGenDirectoryName);
}

void SpriteSheetGenManager::GenerateSpriteSheetGenThumbnail(const stringType& uuidString, const pathType& thumbnailPath) const
{
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	{
		emitter << YAML::Key << s_uuidKey << YAML::Value << uuidString;
		emitter << YAML::EndMap;
	}
	DASSERT_E(emitter.good());
	std::ofstream ostream(thumbnailPath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

void SpriteSheetGenManager::SaveSpriteSheetGensMap() const
{
	YAML::Emitter emitter;
	emitter << s_spriteSheetGensNode;
	std::ofstream ostream(GetSpriteSheetGenDirectory() / s_spriteSheetGensFileName);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

}
