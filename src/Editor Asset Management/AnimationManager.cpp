#include "AnimationManager.h"
#include "ProgramContext.h"
#include "Log.h"
#include "Path.h"
#include "AnimationStateMachineManager.h"

#include "yaml-cpp/yaml.h"

#include <filesystem>
#include <fstream>



namespace DEditor
{

static YAML::Node s_animationsNode;						// Maps UUID -> Animation path.
static const char* s_nameKey{"Name"};
static const char* s_durationKey{"Duration"};
static const char* s_metachannelsKey{"Metachannels"};
static const char* s_metachannelIdKey{"Id"};
static const char* s_metachannelTimeKey{"Time"};
//
static const char* s_componentsKey{"Components"};
static const char* s_componentNameKey{"Component Name"};
static const char* s_attributesKey{"Attributes"};
static const char* s_attributeNameKey{"Attribute Name"};
static const char* s_attributeKeyframeTypeKey{"Attribute Keyframe Type"};
static const char* s_attributeKeyframeTypeInteger{"Integer"};
static const char* s_attributeKeyframeTypeFloat{"Float"};
static const char* s_attributeComponentsKey{"Attribute Components"};
static const char* s_mainPointKey{"Main Point"};
static const char* s_leftControlPointKey{"Left Control Point"};
static const char* s_rightControlPointKey{"Right Control Point"};
//
static const char* s_keyframesKey = "Keyframes";
static const char* s_animationsFileName = "animations.danimations";
static const char* s_animationDirectory = "animation";
static constexpr float s_defaultAnimationDuration = Animation::s_defaultDuration;	// Seconds.

AnimationManager::AnimationManager()
{
	pathType animationsPath(GetAnimationsPath() / s_animationsFileName);
	std::ofstream ostream(animationsPath, std::ios_base::out | std::ios_base::app);
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	s_animationsNode = YAML::LoadFile(animationsPath.string());
}

void AnimationManager::CreateAnimation(const stringType& animationName, const pathType& thumbnailDirectory)
{
	pathType animationPath(GetAnimationsPath() / animationName);
	animationPath += ".danim";
	std::ifstream istream(animationPath);
	if (istream)
	{
		Log::Get().TerminalLog("Animation with name ", animationName.c_str(), " is already created.");
		Log::Get().ConsoleLog(LogLevel::Error, "Animation with name %s is already created.", animationName.c_str());
		istream.close();
		return;
	}
	istream.close();
	std::ofstream ostream(animationPath);
	DASSERT_E(ostream);
	uuidType uuid;
	DCore::UUIDGenerator::Get().GenerateUUID(uuid);
	const stringType uuidString(((stringType)uuid).c_str());
	s_animationsNode[uuidString.c_str()] = Path::Get().MakePathRelativeToAssetsDirectory(animationPath).string();
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	emitter << YAML::Key << s_nameKey << YAML::Value << animationName;
	emitter << YAML::Key << s_durationKey << YAML::Value << s_defaultAnimationDuration;
	emitter << YAML::Key << s_metachannelsKey << YAML::BeginSeq << YAML::EndSeq;
	emitter << YAML::Key << s_componentsKey << YAML::Value << YAML::BeginSeq << YAML::EndSeq;
	emitter << YAML::EndMap;
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
	pathType thumbailPath(thumbnailDirectory / animationName);
	thumbailPath += ".dtanim";
	GenerateAnimationThumbnail(thumbailPath, uuidString, animationName);
	SaveAnimationsMap();
}

Animation AnimationManager::LoadAnimation(const uuidType& animationUUID)
{
	std::string uuidString(animationUUID);
	DASSERT_E(s_animationsNode[uuidString]);
	YAML::Node animationNode(YAML::LoadFile((ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_animationsNode[uuidString].as<std::string>()).string()));
	DASSERT_E(animationNode[s_nameKey]);
	DASSERT_E(animationNode[s_durationKey]);
	DASSERT_E(animationNode[s_metachannelsKey]);
	DASSERT_E(animationNode[s_componentsKey]);
	Animation animation(animationUUID, animationNode[s_nameKey].as<stringType>(), animationNode[s_durationKey].as<float>());
	const YAML::Node componentsNode(animationNode[s_componentsKey]);
	for (YAML::const_iterator componentIt(componentsNode.begin()); componentIt != componentsNode.end(); componentIt++)
	{
		const YAML::Node componentNode(*componentIt);
		DASSERT_E(componentNode[s_componentNameKey]);
		DASSERT_E(componentNode[s_attributesKey]);
		const std::string componentName(componentNode[s_componentNameKey].as<std::string>());
		const DCore::ComponentForm* componentForm(DCore::ComponentForms::Get().GetComponentFormWithName(componentName));
		DASSERT_E(componentForm != nullptr);
		const YAML::Node attributesNode(componentNode[s_attributesKey]);
		for (YAML::const_iterator attributeIt(attributesNode.begin()); attributeIt != attributesNode.end(); attributeIt++)
		{
			const YAML::Node attributeNode(*attributeIt);
			DASSERT_E(attributeNode[s_attributeNameKey]);
			DASSERT_E(attributeNode[s_attributeKeyframeTypeKey]);
			DASSERT_E(attributeNode[s_attributeComponentsKey]);
			const std::string attributeName(attributeNode[s_attributeNameKey].as<std::string>());
			DCore::AttributeIdType attributeId(0);
			for (DCore::AttributeIdType index(0); index < componentForm->SerializedAttributes.size(); index++)
			{
				if (componentForm->SerializedAttributes[index].GetAttributeName().GetName() == attributeName)
				{
					attributeId = index;
					break;
				}
			}
			const DCore::AttributeKeyframeType keyframeType(componentForm->SerializedAttributes[attributeId].GetKeyframeType());
			DASSERT_E(keyframeType != DCore::AttributeKeyframeType::NotDefined);
			switch (keyframeType)
			{
			case DCore::AttributeKeyframeType::Float:
				animation.MakeAnimation(componentForm->Id, attributeId);
				break;
			case DCore::AttributeKeyframeType::Integer:
				animation.MakeAnimation(componentForm->Id, attributeId);
				break;
			default:
				continue;
			}
			YAML::Node attributeComponentsNode(attributeNode[s_attributeComponentsKey]);
			size_t attributeComponentId(0);
			for (YAML::const_iterator attributeComponentIt(attributeComponentsNode.begin()); attributeComponentIt != attributeComponentsNode.end(); attributeComponentIt++)
			{
				const YAML::Node attributeComponentNode(*attributeComponentIt);
				size_t keyframeId(0);
				for (YAML::const_iterator attributeComponentKeyframeIt(attributeComponentNode.begin()); attributeComponentKeyframeIt != attributeComponentNode.end(); attributeComponentKeyframeIt++)
				{
					const YAML::Node attributeComponentKeyframeNode(*attributeComponentKeyframeIt);
					DASSERT_E(attributeComponentKeyframeNode[s_mainPointKey]);
					DASSERT_E(attributeComponentKeyframeNode[s_mainPointKey][0]);
					DASSERT_E(attributeComponentKeyframeNode[s_mainPointKey][1]);
					const DCore::DVec2 mainPoint(attributeComponentKeyframeNode[s_mainPointKey][0].as<float>(), attributeComponentKeyframeNode[s_mainPointKey][1].as<float>());
					switch (keyframeType)
					{
					case DCore::AttributeKeyframeType::Float:
					{
						DASSERT_E(attributeComponentKeyframeNode[s_leftControlPointKey]);
						DASSERT_E(attributeComponentKeyframeNode[s_leftControlPointKey][0]);
						DASSERT_E(attributeComponentKeyframeNode[s_leftControlPointKey][1]);
						DASSERT_E(attributeComponentKeyframeNode[s_rightControlPointKey]);
						DASSERT_E(attributeComponentKeyframeNode[s_rightControlPointKey][0]);
						DASSERT_E(attributeComponentKeyframeNode[s_rightControlPointKey][1]);
						const DCore::DVec2 leftControlPoint(attributeComponentKeyframeNode[s_leftControlPointKey][0].as<float>(), attributeComponentKeyframeNode[s_leftControlPointKey][1].as<float>());
						const DCore::DVec2 rightControlPoint(attributeComponentKeyframeNode[s_rightControlPointKey][0].as<float>(), attributeComponentKeyframeNode[s_rightControlPointKey][1].as<float>());
						FloatKeyframe keyframe(mainPoint.x, mainPoint.y);
						keyframe.SetLeftControlPoint(leftControlPoint.x, leftControlPoint.y);
						keyframe.SetRightControlPoint(rightControlPoint.x, rightControlPoint.y);
						if (keyframeId == 0)
						{
							animation.SetFloatKeyframeAtIndex(componentForm->Id, attributeId, attributeComponentId, 0, keyframe);
						}
						else
						{
							KeyframeAddResult result(animation.TryAddFloatKeyframe(componentForm->Id, attributeId, attributeComponentId, keyframe));
							DASSERT_E(result == KeyframeAddResult::Ok);
						}
						break;
					}
					case DCore::AttributeKeyframeType::Integer:
					{
						IntegerKeyframe keyframe(mainPoint.x, mainPoint.y);
						if (keyframeId == 0)
						{
							animation.SetIntegerKeyframeAtIndex(componentForm->Id, attributeId, attributeComponentId, keyframeId, keyframe);
						}
						else
						{
							KeyframeAddResult result(animation.TryAddIntegerKeyframe(componentForm->Id, attributeId, attributeComponentId, keyframe));
							DASSERT_E(result == KeyframeAddResult::Ok);
						}
						break;
					}
					default:
						continue;
					}
					keyframeId++;
				}
				attributeComponentId++;
			}
		}
	}
	const YAML::Node metachannelsNode(animationNode[s_metachannelsKey]);
	for (size_t i(0); i < metachannelsNode.size(); i++)
	{
		 animation.AddMetachannel();
	}
	size_t index(0);
	animation.IterateOnMetachannels
	(
		[&](auto metachannel) -> bool
		{
			const YAML::Node metachannelNode(metachannelsNode[index++]);
			DASSERT_E(metachannelNode[s_metachannelIdKey] && metachannelNode[s_metachannelTimeKey]);
			size_t metachannelId(metachannelNode[s_metachannelIdKey].as<size_t>());
			float metachannelTime(metachannelNode[s_metachannelTimeKey].as<float>());
			metachannel->SetId(metachannelId);
			metachannel->SetTime(metachannelTime);
			return false;
		}
	);
	return animation;
}

void AnimationManager::SaveChanges(const Animation& animation)
{
	const DCore::UUIDType& uuid(animation.GetUUID());
	const std::string uuidString(uuid);
	DASSERT_E(s_animationsNode[uuidString]);
	const pathType path(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_animationsNode[uuidString].as<stringType>());
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	{
		emitter << YAML::Key << s_nameKey << YAML::Value << animation.GetName().c_str();
		emitter << YAML::Key << s_durationKey << YAML::Value << animation.GetDuration();
		emitter << YAML::Key << s_metachannelsKey << YAML::Value << YAML::BeginSeq;
		{
			animation.IterateOnMetachannels
			(
				[&](auto metachannel) -> bool
				{
					emitter << YAML::BeginMap;
					{
						emitter << YAML::Key << s_metachannelIdKey << YAML::Value << metachannel->GetId();	
						emitter << YAML::Key << s_metachannelTimeKey << YAML::Value << metachannel->GetTime();	
						emitter << YAML::EndMap;
					}
					return false;
				}
			);
			emitter << YAML::EndSeq;
		}
		emitter << YAML::Key << s_componentsKey << YAML::Value << YAML::BeginSeq;
		{
			animation.IterateOnComponentIdsAndAttributeIds
			(
				[&](DCore::ComponentIdType componentId, const std::vector<DCore::AttributeIdType>& attributeIds) -> bool const
				{
					const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
					emitter << YAML::BeginMap;
					{
						emitter << YAML::Key << s_componentNameKey << YAML::Value << componentForm.Name.c_str();
						emitter << YAML::Key << s_attributesKey << YAML::Value << YAML::BeginSeq;
						{
							for (DCore::AttributeIdType attributeId : attributeIds)
							{
								const DCore::AttributeKeyframeType keyframeType(componentForm.SerializedAttributes[attributeId].GetKeyframeType());
								DASSERT_E(keyframeType != DCore::AttributeKeyframeType::NotDefined);
								emitter << YAML::BeginMap;
								{
									emitter << YAML::Key << s_attributeNameKey << YAML::Value << componentForm.SerializedAttributes[attributeId].GetAttributeName().GetName().c_str();
									emitter << YAML::Key << s_attributeKeyframeTypeKey << YAML::Value << (keyframeType == DCore::AttributeKeyframeType::Integer ? s_attributeKeyframeTypeInteger : s_attributeKeyframeTypeFloat);
									emitter << YAML::Key << s_attributeComponentsKey << YAML::Value << YAML::BeginSeq;
									{
										for (size_t attributeComponentId(0); attributeComponentId < componentForm.SerializedAttributes[attributeId].GetNumberOfAttributeComponents(); attributeComponentId++)
										{
											emitter << YAML::BeginSeq;
											{
												switch (keyframeType)
												{
												case DCore::AttributeKeyframeType::Float:
												{
													const std::vector<FloatKeyframe>& keyframes(animation.GetFloatKeyframes(componentId, attributeId, attributeComponentId));
													for (const FloatKeyframe& keyframe : keyframes)
													{
														emitter << YAML::BeginMap;
														{
															const DCore::DVec2 mainPoint(keyframe.GetMainPoint());
															const DCore::DVec2 leftControlPoint(keyframe.GetLeftControlPoint());
															const DCore::DVec2 rightControlPoint(keyframe.GetRightControlPoint());
															emitter << YAML::Key << s_mainPointKey << YAML::Value << YAML::Flow <<  YAML::BeginSeq << mainPoint.x << mainPoint.y << YAML::EndSeq;
															emitter << YAML::Key << s_leftControlPointKey << YAML::Value << YAML::Flow << YAML::BeginSeq << leftControlPoint.x << leftControlPoint.y << YAML::EndSeq;
															emitter << YAML::Key << s_rightControlPointKey << YAML::Value << YAML::Flow << YAML::BeginSeq << rightControlPoint.x << rightControlPoint.y << YAML::EndSeq;
															emitter << YAML::EndMap;
														}
													}
													break;
												}
												case DCore::AttributeKeyframeType::Integer:
												{
													const std::vector<IntegerKeyframe>& keyframes(animation.GetIntegerKeyframes(componentId, attributeId, attributeComponentId));
													for (const IntegerKeyframe& keyframe : keyframes)
													{
														emitter << YAML::BeginMap;
														{
															const DCore::DVec2 mainPoint(keyframe.GetMainPoint());
															emitter << YAML::Key << s_mainPointKey << YAML::Value << YAML::Flow << YAML::BeginSeq << mainPoint.x << mainPoint.y << YAML::EndSeq;
															emitter << YAML::EndMap;
														}
													}
													break;
												}
												default:
													continue;
												}
												emitter << YAML::EndSeq;
											}
										}
										emitter << YAML::EndSeq;
									}
									emitter << YAML::EndMap;
								}
							}
							emitter << YAML::EndSeq;
						}
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
	std::ofstream fstream(path);
	DASSERT_E(fstream.good());
	fstream << emitter.c_str();
	fstream.close();
	DASSERT_E(fstream);
}

void AnimationManager::DeleteAnimation(const uuidType& animationUUID)
{
	const stringType uuidString(animationUUID);
	DASSERT_E(s_animationsNode[uuidString]);
	const pathType animationPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_animationsNode[uuidString].as<stringType>());
	std::filesystem::remove(animationPath);
	YAML::Node newAnimationsNode;
	for (YAML::const_iterator it(s_animationsNode.begin()); it != s_animationsNode.end(); it++)
	{
		const uuidType uuid(it->first.as<stringType>());
		if (uuid == animationUUID)
		{
			continue;
		}
		newAnimationsNode[it->first] = it->second;
	}
	s_animationsNode = newAnimationsNode;
	YAML::Emitter emitter;
	emitter << s_animationsNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(GetAnimationsPath() / s_animationsFileName);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
	if (DCore::AssetManager::Get().IsAnimationLoaded(animationUUID))
	{
		DCore::AssetManager::Get().UnloadAnimation(animationUUID, true);
	}
	AnimationStateMachineManager::Get().RemoveAnimationReferences(animationUUID);
}

AnimationManager::coreAnimationRefType AnimationManager::LoadCoreAnimation(const uuidType& uuid)
{
	using coreAnimationType = DCore::Animation;
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsAnimationLoaded(uuid))
		{
			return DCore::AssetManager::Get().GetAnimation(uuid);
		}
	}
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, m_lockData);
		if (m_animationsLoading.count(uuid) == 0)
		{
			m_animationsLoading.insert(uuid);
			goto LoadAnimation;
		}
	}
	while (true)
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
		if (DCore::AssetManager::Get().IsAnimationLoaded(uuid))
		{
			return DCore::AssetManager::Get().GetAnimation(uuid);
		}
	}
LoadAnimation:
	Animation editorAnimation(LoadAnimation(uuid));
	coreAnimationType coreAnimation(editorAnimation.GenerateCoreAnimation());
	coreAnimationRefType coreAnimationRef(DCore::AssetManager::Get().LoadAnimation(uuid, std::move(coreAnimation)));
	DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, m_lockData);
	m_animationsLoading.erase(uuid);
	return coreAnimationRef;
}

bool AnimationManager::RenameAnimation(const uuidType& uuid, const stringType& newName)
{
	for (YAML::const_iterator it(s_animationsNode.begin()); it != s_animationsNode.end(); it++)
	{
		YAML::Node animationNode(YAML::LoadFile((ProgramContext::Get().GetProjectAssetsDirectoryPath() / it->second.as<stringType>()).string()));
		DASSERT_E(animationNode[s_nameKey]);
		if (animationNode[s_nameKey].as<stringType>() == newName)
		{
			Log::Get().TerminalLog("There is already a animation with name %s.", newName.c_str());
			Log::Get().ConsoleLog(LogLevel::Error, "There is already a animation with name %s.", newName.c_str());
			return false;
		}
	}
	const stringType uuidString(uuid);
	DASSERT_E(s_animationsNode[uuidString]);
	const pathType oldPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_animationsNode[uuidString].as<stringType>());
	YAML::Node animationNode(YAML::LoadFile(oldPath.string()));
	DASSERT_E(animationNode[s_nameKey]);
	animationNode[s_nameKey] = newName;
	YAML::Emitter emitter;
	emitter << animationNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(oldPath);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	pathType newPath(oldPath);
	newPath.replace_filename(newName + ".danim");
	std::filesystem::rename(oldPath, newPath);
	s_animationsNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(newPath).string();
	SaveAnimationsMap();
	DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
	if (DCore::AssetManager::Get().IsAnimationLoaded(uuid))
	{
		DCore::AssetManager::Get().RenameAnimation(uuid, newName);
	}
	return true;
}

AnimationManager::pathType AnimationManager::GetAnimationsPath() const
{
	pathType animationsPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_animationDirectory);
	return animationsPath;
}

void AnimationManager::GenerateAnimationThumbnail(const pathType& thumbailPath, const stringType& uuidString, const stringType& animationName)
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

void AnimationManager::SaveAnimationsMap()
{
	pathType animationsFilePath(GetAnimationsPath() / s_animationsFileName);
	std::ofstream ostream(animationsFilePath);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << s_animationsNode;
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

}
