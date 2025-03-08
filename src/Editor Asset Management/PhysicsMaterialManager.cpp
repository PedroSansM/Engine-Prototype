#include "PhysicsMaterialManager.h"
#include "ProgramContext.h"
#include "Log.h"
#include "Path.h"
#include "SceneManager.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <fstream>



namespace DEditor
{

static YAML::Node s_physicsMaterialsNode;										// Maps UUID -> Physics material path
static const char* s_physicsMaterialsDirectory{"physics material"};
static const char* s_physicsMaterialsFileName{"physicsmaterials.dphysmats"};
static const char* s_densityKey{"Density"};
static const char* s_frictionKey{"Friction"};
static const char* s_restitutionKey{"Restitution"};

PhysicsMaterialManager::PhysicsMaterialManager()
{
	pathType animationsPath(GetPhysicsMaterialsPath() / s_physicsMaterialsFileName);
	std::ofstream ostream(animationsPath, std::ios_base::out | std::ios_base::app);
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	s_physicsMaterialsNode = YAML::LoadFile(animationsPath.string());
}

void PhysicsMaterialManager::CreatePhysicsMaterial(const stringType& physicsMaterialName, const pathType& thumbnailDirectoryPath)
{
	pathType physicslMaterialPath(GetPhysicsMaterialsPath() / physicsMaterialName);
	physicslMaterialPath += ".dphysmat";
	std::ifstream istream(physicslMaterialPath);
	if (istream)
	{
		Log::Get().TerminalLog("Physics material with name ", physicsMaterialName.c_str(), " is already created.");
		Log::Get().ConsoleLog(LogLevel::Error, "Physics material with name %s is already created.", physicsMaterialName.c_str());
		istream.close();
		return;
	}
	istream.close();
	std::ofstream ostream(physicslMaterialPath);
	DASSERT_E(ostream);
	uuidType uuid;
	DCore::UUIDGenerator::Get().GenerateUUID(uuid);
	const stringType uuidString(((stringType)uuid).c_str());
	s_physicsMaterialsNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(physicslMaterialPath).string();
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	emitter << YAML::Key << s_densityKey << YAML::Value << physicsMaterialRefType::defaultDensity;
	emitter << YAML::Key << s_frictionKey << YAML::Value << physicsMaterialRefType::defaultFriction;
	emitter << YAML::Key << s_restitutionKey << YAML::Value << physicsMaterialRefType::defaultRestitution;
	emitter << YAML::EndMap;
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	pathType thumbailPath(thumbnailDirectoryPath / physicsMaterialName);
	thumbailPath += ".dtphysmat";
	GeneratePhysicsMaterialThumbnail(thumbailPath, uuidString, physicsMaterialName);
	SavePhysicsMaterialMap();
}

PhysicsMaterialManager::physicsMaterialRefType PhysicsMaterialManager::LoadPhysicsMaterial(const uuidType& uuid)
{
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsPhysicsMaterialLoaded(uuid))
		{
			return DCore::AssetManager::Get().GetPhysicsMaterial(uuid);
		}
	}
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, m_lockData);
		if (m_loadingPhysicsMaterials.count(uuid) == 0)
		{
			m_loadingPhysicsMaterials.insert(uuid);
			goto LoadPhysicsMaterial;
		}
	}
	while (true)
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsPhysicsMaterialLoaded(uuid))
		{
			return DCore::AssetManager::Get().GetPhysicsMaterial(uuid);
		}
	}
LoadPhysicsMaterial:
	stringType uuidString(uuid);
	DASSERT_E(s_physicsMaterialsNode[uuidString]);
	YAML::Node physicsMaterialNode(YAML::LoadFile((ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_physicsMaterialsNode[uuidString].as<stringType>()).string()));
	DASSERT_E(physicsMaterialNode[s_densityKey]);
	DASSERT_E(physicsMaterialNode[s_frictionKey]);
	DASSERT_E(physicsMaterialNode[s_restitutionKey]);
	const float density(physicsMaterialNode[s_densityKey].as<float>());
	const float friction(physicsMaterialNode[s_frictionKey].as<float>());
	const float restitution(physicsMaterialNode[s_restitutionKey].as<float>());
	DCore::PhysicsMaterial physicsMaterial(density, friction, restitution);
	physicsMaterial.SetName(pathType(s_physicsMaterialsNode[uuidString].as<stringType>()).stem().string());
	DCore::PhysicsMaterialRef physicsMaterialRef(DCore::AssetManager::Get().LoadPhysicsMaterial(uuid, std::move(physicsMaterial)));
	DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, m_lockData);
	m_loadingPhysicsMaterials.erase(uuid);
	return physicsMaterialRef;
}

void PhysicsMaterialManager::SaveChanges(const physicsMaterialRefType physicsMaterial)
{
	// TODO. Atrualizar o asset em memória volátil.
	const uuidType& uuid(physicsMaterial.GetUUID());
	const stringType& uuidString(uuid);
	DASSERT_E(s_physicsMaterialsNode[uuidString]);
	const pathType path(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_physicsMaterialsNode[uuidString].as<stringType>());
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	{
		emitter << YAML::Key << s_densityKey << YAML::Value << physicsMaterial.GetDensity();
		emitter << YAML::Key << s_frictionKey << YAML::Value << physicsMaterial.GetFriction();
		emitter << YAML::Key << s_restitutionKey << YAML::Value << physicsMaterial.GetRestitution();
		emitter << YAML::EndMap;
	};
	DASSERT_E(emitter.good());
	std::ofstream fstream(path);
	DASSERT_E(fstream.good());
	fstream << emitter.c_str();
	fstream.close();
	DASSERT_E(fstream);
}

void PhysicsMaterialManager::DeletePhysicsMaterial(const uuidType& phyiscsMaterialUUID)
{
	const stringType uuidString(phyiscsMaterialUUID);
	DASSERT_E(s_physicsMaterialsNode[uuidString]);
	const pathType physicsMaterialPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_physicsMaterialsNode[uuidString].as<stringType>());
	std::filesystem::remove(physicsMaterialPath);
	YAML::Node newPhysicsMaterialsNode;
	for (YAML::const_iterator it(s_physicsMaterialsNode.begin()); it != s_physicsMaterialsNode.end(); it++)
	{
		const uuidType uuid(it->first.as<stringType>());
		if (uuid == phyiscsMaterialUUID)
		{
			continue;
		}
		newPhysicsMaterialsNode[it->first] = it->second;
	}
	s_physicsMaterialsNode = newPhysicsMaterialsNode;
	YAML::Emitter emitter;
	emitter << s_physicsMaterialsNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(GetPhysicsMaterialsPath() / s_physicsMaterialsFileName);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsPhysicsMaterialLoaded(phyiscsMaterialUUID))
		{
			DCore::AssetManager::Get().UnloadPhysicsMaterial(phyiscsMaterialUUID, true);
		}
	}
	SceneManager::Get().SaveLoadedScenes();
}

bool PhysicsMaterialManager::RenamePhysicsMaterial(const uuidType& uuid, const stringType& newName)
{
	for (YAML::const_iterator it(s_physicsMaterialsNode.begin()); it != s_physicsMaterialsNode.end(); it++)
	{
		const pathType physicsMaterialPath(it->second.as<stringType>());
		if (physicsMaterialPath.stem() == newName)
		{
			Log::Get().TerminalLog("There is already a physics material with name %s.", newName.c_str());
			Log::Get().ConsoleLog(LogLevel::Error, "There is already a physics material with name %s.", newName.c_str());
			return false;
		}
	}
	const stringType uuidString(uuid);
	DASSERT_E(s_physicsMaterialsNode[uuidString]);
	const pathType oldPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_physicsMaterialsNode[uuidString].as<stringType>());
	pathType newPath(oldPath);
	newPath.replace_filename(newName + ".dphysmat");
	s_physicsMaterialsNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(newPath).string().c_str();
	SavePhysicsMaterialMap();
	std::filesystem::rename(oldPath, newPath);
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
	if (DCore::AssetManager::Get().IsPhysicsMaterialLoaded(uuid))
	{
		DCore::PhysicsMaterialRef ref(DCore::AssetManager::Get().GetPhysicsMaterial(uuid));
		ref.SetName(newName);
		ref.Unload();
	}
	return true;
}

PhysicsMaterialManager::pathType PhysicsMaterialManager::GetPhysicsMaterialsPath() const
{
	pathType physicsMaterialPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_physicsMaterialsDirectory);
	return physicsMaterialPath;
}

void PhysicsMaterialManager::GeneratePhysicsMaterialThumbnail(const pathType& thumbailPath, const stringType& uuidString, const stringType& physicsMaterialName)
{
	std::ofstream ostream(thumbailPath);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	emitter << YAML::Key << "UUID" << YAML::Value << uuidString.c_str();
	emitter << YAML::EndMap;
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

void PhysicsMaterialManager::SavePhysicsMaterialMap()
{
	pathType physicsMaterialsFilePath(GetPhysicsMaterialsPath() / s_physicsMaterialsFileName);
	std::ofstream ostream(physicsMaterialsFilePath);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << s_physicsMaterialsNode;
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

}
