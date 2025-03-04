#pragma once

#include "EditorAnimationStateMachine.h"
#include "AnimationStateMachinePanel.h"

#include "DommusCore.h"

#include <filesystem>
#include <string>



namespace DEditor
{

class AnimationStateMachineManager
{
public:
	using animationStateMachineType = EditorAnimationStateMachine;	
	using coreAnimationStateMachineType = DCore::AnimationStateMachine;
	using uuidType = DCore::UUIDType;
	using pathType = std::filesystem::path;
	using stringType = std::string;
	using parameterInfoContainerType = AnimationStateMachinePanel::parameterInfoContainerType;
	using parameterInfoType = AnimationStateMachinePanel::ParameterInfo;
	using coreAnimationStateMachineRefType = DCore::AnimationStateMachineRef;
public:
	~AnimationStateMachineManager() = default;
public:
	static AnimationStateMachineManager& Get()
	{
		static AnimationStateMachineManager instance;
		return instance;
	}
public:
	void CreateAnimationStateMachine(const stringType& name, const pathType& thumbnailDirectoryPath);
	EditorAnimationStateMachine LoadAnimationStateMachine(const uuidType&, const stringType& name, parameterInfoContainerType* outParameterInfos = nullptr);
	void SaveChanges(const animationStateMachineType&, const parameterInfoContainerType&);
	stringType GetAnimationStateMachineName(const uuidType&);
	void RemoveAnimationReferences(const uuidType& animationUUID);
	void DeleteAnimationStateMachine(const uuidType& asmUUID);
	bool RenameAnimationStateMachine(const uuidType& uuid, const stringType& newName);
private:
	AnimationStateMachineManager();
private:
	pathType GetAnimationStateMachinesPath() const;
	void GenerateThumbnail(const pathType& path, const stringType& uuidString);
	void SaveAnimationStateMachineMap();
};

}
