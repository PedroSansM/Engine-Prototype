#pragma once

#include "DommusCore.h"

#include "EditorAnimation.h"

#include <filesystem>



namespace DEditor
{

class AnimationManager
{
public:
	using uuidType = DCore::UUIDType;
	using coreAnimationRefType = DCore::AnimationRef;
	using stringType = std::string;
	using pathType = std::filesystem::path;
public:
	~AnimationManager() = default;
public:
	static AnimationManager& Get()
	{
		static AnimationManager animationManager;
		return animationManager;
	}
public:
	void CreateAnimation(const stringType& animationName, const pathType& thumbnailDirectory);
	Animation LoadAnimation(const uuidType& animationUUID);
	[[nodiscard]] coreAnimationRefType LoadCoreAnimation(const uuidType& animationUUID);
	void SaveChanges(const Animation&);
	void DeleteAnimation(const uuidType&);
	bool RenameAnimation(const uuidType&, const stringType& newName);
private:
	AnimationManager();
private:
	pathType GetAnimationsPath() const;
	void GenerateAnimationThumbnail(const pathType& thumbailPath, const stringType& uuidString, const stringType& animationName);
	void SaveAnimationsMap();
};

}
