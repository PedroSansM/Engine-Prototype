#pragma once

#include "SpriteSheetGen.h"

#include "DommusCore.h"

#include <filesystem>
#include <string>



namespace DEditor
{

class SpriteSheetGenManager
{
public:
	using stringType = std::string;
	using uuidType = DCore::UUIDType;
	using spriteSheetGenContainerType = DCore::ReciclingVector<SpriteSheetGen>;
	using spriteSheetGenRefType = spriteSheetGenContainerType::Ref;
	using pathType = std::filesystem::path;
public:
	static constexpr size_t maxNumberOfSpriteSheetGensOpened{10};
public:
	static const char* spriteSheetGenFileExtension;
	static const char* spriteSheetGenThumbnailExtension;
	static const char* spriteSheetGenDirectoryName;
public:
	SpriteSheetGenManager(const SpriteSheetGenManager&) = delete;
	SpriteSheetGenManager(SpriteSheetGenManager&&) = delete;
	~SpriteSheetGenManager();
public:
	static SpriteSheetGenManager& Get()
	{
		static SpriteSheetGenManager instance;
		return instance;
	}
public:
	void CreateSpriteSheetGen(const stringType& name, const pathType& thumbnailDirectory);
	bool IsSpriteSheetGenLoaded(const uuidType&);
	spriteSheetGenRefType LoadSpriteSheetGen(const uuidType&);
	bool RenameSpriteSheetGen(const uuidType&, const stringType& newName);
	void UnloadSpriteSheetGen(spriteSheetGenRefType);
	void DeleteSpriteSheetGen(const uuidType&);
	void SaveSpriteSheetGen(spriteSheetGenRefType);
private:
	SpriteSheetGenManager();
private:
	spriteSheetGenContainerType m_spriteSheetGens;
private:
	pathType GetSpriteSheetGenDirectory() const;
	void GenerateSpriteSheetGenThumbnail(const stringType& uuidString, const pathType& thumbnailPath) const;
	void SaveSpriteSheetGensMap() const;
};

}
