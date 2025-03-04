#include "SceneSerialization.h"
#include "MaterialManager.h"
#include "AnimationStateMachineManager.h"
#include "PhysicsMaterialManager.h"
#include "TypeNames.h"
#include "Log.h"

#include "DommusCore.h"

#include "yaml-cpp/yaml.h"
#include "glm/gtc/integer.hpp"

#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>



namespace DEditor
{

SceneSerialization::returnErrorType SceneSerialization::DeserializeScene(const pathType& scenePath, sceneRefType sceneRef)
{
	using entityRefType = DCore::EntityRef;
	using componentFormType = DCore::ComponentForm;
	using uuidComponentType = DCore::UUIDComponent;
	using attributeNameType = DCore::AttributeName;
	using componentFormType = DCore::ComponentForm;
	using attributeType = DCore::AttributeType;
	using nodeType = YAML::Node;
 	nodeType root;
	try 
	{
		nodeType tryRoot(YAML::LoadFile(scenePath.string()));
		root = std::move(tryRoot);
	}
	catch (const std::exception& e) 
	{
		return returnErrorType(false, e.what());
	}
 	nodeType sceneNode(root["Scene"]);
	nodeType sceneNameNode(sceneNode["Name"]);
	nodeType entitiesNode(root["Entities"]);
	if (!sceneNode || !sceneNameNode || !entitiesNode)
	{
		returnErrorType returnError;
		returnError.Ok = false;
		returnError.Message.Append("Bad scene file: ").Append(scenePath.string().c_str()).Append(".");
		return returnError;
	}
	sceneRef.SetName(sceneNameNode.as<std::string>().c_str());
	// Create the entities with the UUID Component only.
	entityRefType* entities(new entityRefType[entitiesNode.size()]);
	const componentFormType& uuidComponentForm(DCore::ComponentForms::Get()[DCore::ComponentId::GetId<uuidComponentType>()]);
	for (size_t index(0); index < entitiesNode.size(); index++)
	{
		nodeType uuidNode(entitiesNode[index][uuidComponentForm.Name]);
		const auto& uuidComponentAttribute(uuidComponentForm.SerializedAttributes[0]);
		const attributeNameType& attributeName(uuidComponentAttribute.GetAttributeName());
		nodeType uuidAttributeNode(uuidNode[attributeName.GetName().c_str()]);
		if (!uuidAttributeNode)
		{
			returnErrorType returnError;
			returnError.Ok = false;
			returnError.Message.Append("Fail to deserialize uuid component in scene in the path: ").Append(scenePath.string().c_str()).Append(".");
			return returnError;
		}
		DCore::ConstructorArgs<DCore::UUIDComponent> args{uuidNode[attributeName.GetName()].as<std::string>()};
		new (&entities[index]) entityRefType(sceneRef.CreateEntity<DCore::UUIDComponent>(std::make_tuple(args)), sceneRef);
	}
	size_t entityIndex(0);
	for (YAML::const_iterator entityIt(entitiesNode.begin()); entityIt != entitiesNode.end(); entityIt++)
	{
		size_t totalSerializedSize(0);
		size_t componentIndex(0);
		const size_t numberOfComponents(entityIt->size());
		// Recall that uuidComponent must not be included in componentIds and componentSizes.
		DCore::ComponentIdType* componentIds(static_cast<DCore::ComponentIdType*>(new DCore::ComponentIdType[numberOfComponents - 1]));
		size_t* componentSizes(static_cast<size_t*>(new size_t[numberOfComponents - 1]));
		for (YAML::const_iterator componentIt(entityIt->begin()); componentIt != entityIt->end(); componentIt++)
		{
			const stringType componentName(componentIt->first.as<std::string>());
			const componentFormType* componentForm(DCore::ComponentForms::Get().GetComponentFormWithName(componentName));
			if (componentForm == nullptr)
			{
				returnErrorType returnError;
				returnError.Ok = false;
				returnError.Message.Append("Fail to deserialize component with name ").Append(componentName.c_str()).Append(" in scene file: ").Append(scenePath.string().c_str()).Append(".");
				return returnError;
			}
			if (*componentForm == uuidComponentForm)
			{
				continue;
			}
			totalSerializedSize += componentForm->SerializedSize;
			componentIds[componentIndex] = componentForm->Id;
			componentSizes[componentIndex] = componentForm->TotalSize;
			componentIndex++;
		}
		componentIndex = 0;
		size_t argsBufferOffset(0);
		size_t componentFormRefsOffset(0);
		char* argsBuffer(new char[totalSerializedSize]);
		for (YAML::const_iterator componentIt(entityIt->begin()); componentIt != entityIt->end(); componentIt++)
		{
			if (componentIt->first.as<stringType>() == DCore::ComponentForms::Get()[DCore::ComponentId::GetId<DCore::UUIDComponent>()].Name)
			{
				continue;
			}
			size_t componentArgsOffset(argsBufferOffset);
			nodeType attributeMap(componentIt->second);
			const componentFormType& componentForm(DCore::ComponentForms::Get()[componentIds[componentIndex++]]);
			for (const auto& attributeInfo : componentForm.SerializedAttributes)
			{
				const attributeNameType& attributeName(attributeInfo.GetAttributeName());
				const attributeType attributeType(attributeInfo.GetAttributeType());
				if (!attributeMap[attributeName.GetName().c_str()])
				{
					continue;
				}
				switch (attributeType) 
				{
				case attributeType::Integer:
				{
					*((DCore::DInt*)&argsBuffer[argsBufferOffset]) = attributeMap[attributeName.GetName().c_str()].as<DCore::DInt>();
					argsBufferOffset += sizeof(DCore::DInt);
					break;
				}
				case attributeType::UInteger:	
				{
					*((DCore::DUInt*)&argsBuffer[argsBufferOffset]) = attributeMap[attributeName.GetName().c_str()].as<DCore::DUInt>();
					argsBufferOffset += sizeof(DCore::DUInt);
					break;
				}
				case attributeType::Float:
				{
					*((DCore::DFloat*)&argsBuffer[argsBufferOffset]) = attributeMap[attributeName.GetName().c_str()].as<DCore::DFloat>();
					argsBufferOffset += sizeof(DCore::DFloat);
					break;
				}
				case attributeType::Size:
				{
					*((DCore::DSize*)&argsBuffer[argsBufferOffset]) = attributeMap[attributeName.GetName().c_str()].as<DCore::DSize>();
					argsBufferOffset += sizeof(DCore::DSize);
					break;
				}
				case attributeType::Logic:
				{
					*((DCore::DLogic*)&argsBuffer[argsBufferOffset]) = attributeMap[attributeName.GetName().c_str()].as<DCore::DLogic>();
					argsBufferOffset += sizeof(DCore::DLogic);
					break;
				}
				case attributeType::Vector2:
				{
					YAML::Node vector2Node(attributeMap[attributeName.GetName().c_str()]);
					DCore::DVec2 vec2(vector2Node[0].as<float>(), vector2Node[1].as<float>());
					*((DCore::DVec2*)&argsBuffer[argsBufferOffset]) = vec2;
					argsBufferOffset += sizeof(DCore::DVec2);
					break;
				}
				case attributeType::UIVector2:
				{
					YAML::Node vector2Node(attributeMap[attributeName.GetName().c_str()]);
					DCore::DVec2 vec2(vector2Node[0].as<float>(), vector2Node[1].as<float>());
					*((DCore::DVec2*)&argsBuffer[argsBufferOffset]) = vec2;
					argsBufferOffset += sizeof(DCore::DVec2);
					break;
				}
				case attributeType::Vector3:
				{
					YAML::Node vector3Node(attributeMap[attributeName.GetName().c_str()]);
					DCore::DVec3 vec3(vector3Node[0].as<float>(), vector3Node[1].as<float>(), vector3Node[2].as<float>());
					*((DCore::DVec3*)&argsBuffer[argsBufferOffset]) = vec3;
					argsBufferOffset += sizeof(DCore::DVec3);
					break;
				}
				case attributeType::String:
				{
					std::strcpy(&argsBuffer[argsBufferOffset], attributeMap[attributeName.GetName().c_str()].as<std::string>().c_str());
					argsBufferOffset += sizeof(DCore::DString); 
					break;
				}
				case attributeType::EntityReference:
				{
					std::string uuidString(attributeMap[attributeName.GetName().c_str()].as<std::string>());
					if (uuidString.empty())
					{
						new (&argsBuffer[argsBufferOffset]) DCore::EntityRef();;
					}
					DCore::UUIDType uuid(uuidString);
					DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *sceneRef.GetLockData());
					sceneRef.Iterate<DCore::UUIDComponent>
					(
						[&](DCore::Entity entity, DCore::ComponentRef<DCore::UUIDComponent> uuidComponent) -> bool
						{
							DCore::UUIDType entityUUID;
							uuidComponent.GetUUID(entityUUID);
							if (entityUUID != uuid)
							{
								return false;
							}
							new (&argsBuffer[argsBufferOffset]) DCore::EntityRef(entity, sceneRef);
							return true;
						}
					);
					argsBufferOffset += sizeof(DCore::EntityRef);
					break;
				}
				case attributeType::SpriteMaterial:
				{
					const stringType spriteMaterialUUIDString(attributeMap[attributeName.GetName().c_str()].as<std::string>());
					DCore::SpriteMaterialRef spriteMaterialRef;
					if (!spriteMaterialUUIDString.empty())
					{
						spriteMaterialRef = MaterialManager::Get().LoadSpriteMaterial(DCore::UUIDType(spriteMaterialUUIDString));
					}
					new (&argsBuffer[argsBufferOffset]) DCore::SpriteMaterialRef(spriteMaterialRef);
					argsBufferOffset += sizeof(DCore::SpriteMaterialRef);
					break;
				}
				case attributeType::TaggedList:
				{
					const stringType tagName(attributeMap[attributeName.GetName().c_str()].as<stringType>());
					size_t tagIndex(0);
					for (size_t index(0); index < attributeName.GetNumberOfComponents(); index++)
					{
						if (tagName == attributeName.GetComponentAtIndex(index))
						{
							tagIndex = index;
							break;
						}
					}
					new (&argsBuffer[argsBufferOffset]) size_t(tagIndex);
					argsBufferOffset += sizeof(size_t);
					break;
				}
				case attributeType::Color:
				{
					YAML::Node colorNode(attributeMap[attributeName.GetName().c_str()]);
					DCore::DVec4 vec4(colorNode[0].as<float>(), colorNode[1].as<float>(), colorNode[2].as<float>(), colorNode[3].as<float>());
					new (&argsBuffer[argsBufferOffset]) DCore::DVec4(vec4);
					argsBufferOffset += sizeof(DCore::DVec4);
					break;
				}
				case attributeType::AnimationStateMachine:
				{
					using coreAnimationStateMachineType = DCore::AnimationStateMachine;
					using coreAnimationStateMachineRefType = DCore::AnimationStateMachineRef;
					const stringType animationStateMachineUUIDString(attributeMap[attributeName.GetName().c_str()].as<stringType>());
					coreAnimationStateMachineRefType animationStateMachineRef;
					if (!animationStateMachineUUIDString.empty())
					{
						const uuidType animationStateMachineUUID(animationStateMachineUUIDString);
						const stringType animationStateMachineName(AnimationStateMachineManager::Get().GetAnimationStateMachineName(animationStateMachineUUID));
						coreAnimationStateMachineType animationStateMachine(std::move(AnimationStateMachineManager::Get().LoadAnimationStateMachine(animationStateMachineUUID, animationStateMachineName).GetCoreAnimationStateMachine()));
						animationStateMachineRef = DCore::AssetManager::Get().AddAnimationStateMachine(std::move(animationStateMachine));
						DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationStateMachineAssetManager*>(&DCore::AssetManager::Get()));
						DASSERT_E(animationStateMachineRef.IsValid());
						animationStateMachineRef.AttachTo(entities[entityIndex]);
					}
					new (&argsBuffer[argsBufferOffset]) coreAnimationStateMachineRefType(animationStateMachineRef);
					argsBufferOffset += sizeof(coreAnimationStateMachineRefType);
					break;
				}
				case attributeType::PhysicsBodyType:
				{
					const stringType bodyTypeName(attributeMap[attributeName.GetName()].as<stringType>());
					DCore::DBodyType bodyType;
					if (bodyTypeName == "Static")
					{
						bodyType = DCore::DBodyType::Static;
					}
					else if (bodyTypeName == "Kinematic")
					{
						bodyType = DCore::DBodyType::Kinematic;
					}
					else if (bodyTypeName == "Dynamic")
					{
						bodyType = DCore::DBodyType::Dynamic;
					}
					else
					{
						DASSERT_E(false);
					}
					new (&argsBuffer[argsBufferOffset]) DCore::DBodyType(bodyType);
					argsBufferOffset += sizeof(DCore::DBodyType);
					break;
				}
				case attributeType::PhysicsMaterial:
				{
					using physicsMaterialRefType = DCore::PhysicsMaterialRef;
					const stringType physicsMaterialUUIDString(attributeMap[attributeName.GetName().c_str()].as<stringType>());
					physicsMaterialRefType physicsMaterial;
					if (!physicsMaterialUUIDString.empty())
					{
						const uuidType physicsMaterialUUID(attributeMap[attributeName.GetName().c_str()].as<stringType>());
						physicsMaterial = PhysicsMaterialManager::Get().LoadPhysicsMaterial(physicsMaterialUUID);
					}
					new (&argsBuffer[argsBufferOffset]) physicsMaterialRefType(physicsMaterial);
					argsBufferOffset += sizeof(physicsMaterialRefType);
					break;	
				}
				case attributeType::PhysicsLayer:
				{
					using physicsLayerType = DCore::Physics::PhysicsLayer;
					const std::string physicsLayerTypeName(attributeMap[attributeName.GetName().c_str()].as<stringType>());
					physicsLayerType physicsLayer;
					for (size_t i(0); i < DCore::Physics::numberOfPhysicsLayers; i++)
					{
						if (std::strcmp(physicsLayerTypeName.c_str(), TypeNames::physicsLayerNames[i]) == 0)
						{
							physicsLayer = static_cast<physicsLayerType>(i == 0 ? 0 : 1<<(i-1));
							break;
						}
					}
					new (&argsBuffer[argsBufferOffset]) physicsLayerType(physicsLayer);
					argsBufferOffset += sizeof(physicsLayerType);
					break;
				}
				case attributeType::PhysicsLayers:
				{
					using physicsLayerType = DCore::Physics::PhysicsLayer;
					physicsLayerType physicsLayers(static_cast<physicsLayerType>(0));
					YAML::Node physicsLayersNode(attributeMap[attributeName.GetName()]);
					for (YAML::const_iterator it(physicsLayersNode.begin()); it != physicsLayersNode.end(); it++)
					{
						const stringType physicsLayerName(it->as<stringType>());
						for (size_t i(0); i < DCore::Physics::numberOfPhysicsLayers - 1; i++)
						{
							if (std::strcmp(physicsLayerName.c_str(), TypeNames::physicsLayerNames[i + 1]) == 0)
							{
								physicsLayers = static_cast<physicsLayerType>(static_cast<uint64_t>(physicsLayers) | static_cast<uint64_t>(1)<<i);
							}
						}
					}
					new (&argsBuffer[argsBufferOffset]) physicsLayerType(physicsLayers);
					argsBufferOffset += sizeof(physicsLayerType);
					break;
				}
				case attributeType::SoundEventInstance:
				{
					const std::string eventPath(attributeMap[attributeName.GetName()].as<stringType>());
					DCore::SoundEventInstance eventInstance(eventPath);
					DCore::Sound::Get().IterateOnLoadedBanks
					(
						[&](DCore::Sound::bankType* bank) -> bool
						{
							int count(0);
							FMOD_RESULT result(bank->getEventCount(&count));
							DASSERT_E(result == FMOD_OK);
							if (count == 0)
							{
								return false;
							}
							FMOD::Studio::EventDescription** eventDescriptions(static_cast<FMOD::Studio::EventDescription**>(malloc(count * sizeof(char*))));
							result = bank->getEventList(eventDescriptions, count, nullptr);
							DASSERT_E(result == FMOD_OK);
							for (size_t i(0); i < count; i++)
							{
								FMOD::Studio::EventDescription* eventDescription(eventDescriptions[i]);
								int pathSize(0);
								result = eventDescription->getPath(nullptr, 0, &pathSize);
								DASSERT_E(result == FMOD_OK);
								char* path(static_cast<char*>(malloc(pathSize * sizeof(char))));
								result = eventDescription->getPath(path, pathSize, nullptr);
								DASSERT_E(result == FMOD_OK);
								if (std::strcmp(path, eventPath.c_str()) == 0)
								{
									eventInstance.Internal_SetBank(bank);
									free(eventDescriptions);
									free(path);
									return true;
								}
								free(path);
							}
							free(eventDescriptions);
							return false;
						}
					);
					if (eventInstance.Internal_IsBankNull() && !eventInstance.GetPath().Empty())
					{
						Log::Get().TerminalLog("Sound Event Instance attribute refer to a non-existing sound bank path: %s", eventInstance.GetPath().Data());
						Log::Get().ConsoleLog(LogLevel::Warning, "Sound Event Instance attribute refer to a non-existing sound bank path: %s", eventInstance.GetPath().Data());
					}
					new (&argsBuffer[argsBufferOffset]) DCore::SoundEventInstance(std::move(eventInstance));
					argsBufferOffset += sizeof(DCore::SoundEventInstance);
					break;
				}
				default:
					DASSERT_E(false);
					continue;	
				}
			}
		}
		argsBufferOffset = 0;
		entityRefType entityRef(entities[entityIndex++]);
		entityRef.AddComponents
		(
			componentIds, componentSizes, numberOfComponents - 1,
			[&](DCore::ComponentIdType componentId, void* componentAddress) -> void
			{
				const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
				componentForm.PlacementNewConstructor(componentAddress, &argsBuffer[argsBufferOffset]);
				argsBufferOffset += componentForm.SerializedSize;
			}
		);
		delete[] argsBuffer;
		delete[] componentIds;
		delete[] componentSizes;
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *sceneRef.GetLockData());
		entityRef.IterateOnComponents
		(
			[&](DCore::ComponentIdType componentId, void* componentAddress) -> bool
			{
				const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
				if (componentForm.IsScriptComponent)
				{
					DCore::ReadWriteLockGuard guard(DCore::LockType::WriteLock, *sceneRef.GetLockData());
					DCore::ScriptComponent* scriptComponent(static_cast<DCore::ScriptComponent*>(componentAddress));
					scriptComponent->Setup(entityRef, componentId);
				}
				return false;
			}
		);
	}
	delete[] entities;
	return returnErrorType();
}

SceneSerialization::returnErrorType SceneSerialization::SerializeScene(const pathType& scenePath, sceneRefType sceneRef)
{
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	{
		emitter << YAML::Key << "Scene";
		emitter << YAML::Value << YAML::BeginMap;
		{
			DCore::DString sceneName;
			sceneRef.GetName(sceneName);
			emitter << YAML::Key << "Name" << YAML::Value << sceneName.Data();
			emitter << YAML::EndMap;
		}
		emitter << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		{
			sceneRef.IterateOnEntities
			(
				[&](DCore::Entity entity) -> bool
				{
					emitter << YAML::BeginMap;
					{
						DCore::EntityRef entityRef(entity, sceneRef);
						size_t numberOfComponents(entityRef.GetNumberOfComponents());
						DCore::ComponentIdType* componentIds(new DCore::ComponentIdType[numberOfComponents]);
						entityRef.GetComponentIds(componentIds);
						for (size_t componentIdsOffset(0); componentIdsOffset < numberOfComponents; componentIdsOffset++)
						{
							const DCore::ComponentIdType componentId(componentIds[componentIdsOffset]);
							DCore::ComponentRef<DCore::Component> component(entityRef.GetComponent(componentId));
							const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
							emitter << YAML::Key << componentForm.Name << YAML::Value << YAML::BeginMap;
							{
								for (const DCore::SerializedAttribute& attribute : componentForm.SerializedAttributes)
								{
									const DCore::AttributeName& attributeName(attribute.GetAttributeName());
									DCore::AttributeType attributeType(attribute.GetAttributeType());
									DCore::AttributeIdType attributeId(attribute.GetAttributeId());
									emitter << YAML::Key << attributeName.GetName().c_str();
									switch (attributeType)
									{
									case DCore::AttributeType::Integer:
									{
										DCore::DInt value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::DInt));
										emitter << YAML::Value << value;
										break;
									}
									case DCore::AttributeType::UInteger:
									{
										DCore::DUInt value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::DUInt));
										emitter << YAML::Value << value;
										break;
									}
									case DCore::AttributeType::Float:
									{
										DCore::DFloat value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::DFloat));
										emitter << YAML::Value << value;
										break;
									}
									case DCore::AttributeType::Size:
									{
										DCore::DSize value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::DSize));
										emitter << YAML::Value << value;
										break;
									}
									case DCore::AttributeType::Logic:
									{
										DCore::DLogic value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::DLogic));
										emitter << YAML::Value << value;
										break;
									}
									case DCore::AttributeType::Vector2:
									{
										emitter << YAML::Value << YAML::Flow << YAML::BeginSeq;
										{
											DCore::DVec2 value;
											component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec2));
											emitter << value.x << value.y;
											emitter << YAML::EndSeq;
										}
										break;
									}
									case DCore::AttributeType::UIVector2:
									{
										emitter << YAML::Value << YAML::Flow << YAML::BeginSeq;
										{
											DCore::DVec2 value;
											component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec2));
											emitter << value.x << value.y;
											emitter << YAML::EndSeq;
										}
										break;
									}
									case DCore::AttributeType::Vector3:
									{
										emitter << YAML::Value << YAML::Flow << YAML::BeginSeq;
										{
											DCore::DVec3 value;
											component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec3));
											emitter << value.x << value.y << value.z;
											emitter << YAML::EndSeq;
										}
										break;
									}
									case DCore::AttributeType::String:
									{
										DCore::DString value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::DString));
										emitter << YAML::Value << value.Data();
										break;
									}
									case DCore::AttributeType::UUID:
									{
										DCore::UUIDType value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::UUIDType));
										emitter << YAML::Value << (std::string)value;
										break;
									}
									case DCore::AttributeType::EntityReference:
									{
										DCore::EntityRef entityRef;
										component.GetAttributePtr(attributeId, &entityRef, sizeof(DCore::EntityRef));
										if (entityRef.IsValid())
										{
											DCore::UUIDType uuid;
											entityRef.GetUUID(uuid);
											emitter << YAML::Value << (std::string)uuid;
										}
										else
										{
											emitter << YAML::Value << "";
										}
										break;
									}
									case DCore::AttributeType::SpriteMaterial:
									{
										DCore::SpriteMaterialRef spriteMaterialRef;
										component.GetAttributePtr(attributeId, &spriteMaterialRef, sizeof(DCore::SpriteMaterialRef));
										DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SpriteMaterialAssetManager*>(&DCore::AssetManager::Get()));
										if (spriteMaterialRef.IsValid())
										{
											emitter << YAML::Value << (std::string)spriteMaterialRef.GetUUID();
										}
										else
										{
											emitter << YAML::Value << "";
										}
										break;
									}
									case DCore::AttributeType::TaggedList:
									{
										size_t currentTagIndex;
										component.GetAttributePtr(attributeId, &currentTagIndex, sizeof(size_t));
										const stringType& currentTagName(attributeName.GetComponentAtIndex(currentTagIndex));
										emitter << YAML::Value << currentTagName;
										break;
									}
									case DCore::AttributeType::Color:
									{
										DCore::DVec4 value;
										component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec4));
										emitter << YAML::Value;
										{
											emitter << YAML::Value << YAML::Flow << YAML::BeginSeq;
											{
												emitter << value.r << value.g << value.b << value.a;
												emitter << YAML::EndSeq;
											}
										}
										break;
									}
									case DCore::AttributeType::AnimationStateMachine:
									{
										using coreAnimationStateMachineRefType = DCore::AnimationStateMachineRef;
										coreAnimationStateMachineRefType animationStateMachine;
										component.GetAttributePtr(attributeId, &animationStateMachine, sizeof(coreAnimationStateMachineRefType));
										DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationStateMachineAssetManager*>(&DCore::AssetManager::Get()));
										emitter << YAML::Value << (animationStateMachine.IsValid() ? (stringType)animationStateMachine.GetUUID() : "");
										break;
									}
									case DCore::AttributeType::PhysicsBodyType:
									{
										DCore::DBodyType bodyType;
										component.GetAttributePtr(attributeId, &bodyType, sizeof(DCore::DBodyType));
										switch (bodyType)
										{
										case DCore::DBodyType::Static:
											emitter << YAML::Value << "Static";
											break;
										case DCore::DBodyType::Kinematic:
											emitter << YAML::Value << "Kinematic";
											break;
										case DCore::DBodyType::Dynamic:
											emitter << YAML::Value << "Dynamic";
											break;
										}
										break;
									}
									case DCore::AttributeType::PhysicsMaterial:
									{
										using physicsMaterialRefType = DCore::PhysicsMaterialRef;
										physicsMaterialRefType physicsMaterial;
										component.GetAttributePtr(attributeId, &physicsMaterial, sizeof(physicsMaterialRefType));
										DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
										emitter << YAML::Value << (physicsMaterial.IsValid() ? (stringType)physicsMaterial.GetUUID() : "");
										break;
									}
									case DCore::AttributeType::PhysicsLayer:
									{
										DCore::Physics::PhysicsLayer physicsLayer;
										component.GetAttributePtr(attributeId, &physicsLayer, sizeof(decltype(physicsLayer)));
										emitter << YAML::Value << TypeNames::physicsLayerNames[static_cast<uint64_t>(physicsLayer) == 0 ? 0 : (1 + glm::log2(static_cast<uint64_t>(physicsLayer)))];
										break;
									}
									case DCore::AttributeType::PhysicsLayers:
									{
										DCore::Array<stringType, DCore::Physics::numberOfPhysicsLayers> typeNames;
										DCore::Physics::PhysicsLayer physicsLayers;
										component.GetAttributePtr(attributeId, &physicsLayers, sizeof(decltype(physicsLayers)));
										for (size_t i(0); i < DCore::Physics::numberOfPhysicsLayers - 1; i++)
										{
											if (static_cast<uint64_t>(physicsLayers) & 1<<i)
											{
												typeNames.PushBack(TypeNames::physicsLayerNames[i + 1]);
											}
										}
										emitter << YAML::Value << YAML::BeginSeq;
										{
											for (const stringType& typeName : typeNames)
											{
												emitter << typeName.c_str();
											}
											emitter << YAML::EndSeq;
										}
										break;
									}
									case DCore::AttributeType::SoundEventInstance:
									{
										DCore::SoundEventInstance soundEventInstance;
										component.GetAttributePtr(attributeId, &soundEventInstance, sizeof(DCore::SoundEventInstance));
										emitter << YAML::Value << soundEventInstance.GetPath().Data();
										break;
									}
									default:
										DASSERT_E(false);
										break;
									}
								}
								emitter << YAML::EndMap;
							}
						}
						delete[] componentIds;
						emitter << YAML::EndMap;
						return false;
					}
				}
			);
			emitter << YAML::EndSeq;
		}
		emitter << YAML::EndMap;
	}
	DCore::ReturnError returnError;
	if (!emitter.good())
	{
		returnError.Ok = false;
		returnError.Message.Append("Bad emitter in scene serialization: ").Append(scenePath.string().c_str()).Append(".");
	}
	std::ofstream fileOutStream;
	fileOutStream.open(scenePath.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!fileOutStream)
	{
		returnError.Ok = false;
		returnError.Message.Append("Failure in opening the scene file in path: ").Append(scenePath.string().c_str()).Append(".");
		return returnError;
	}
	fileOutStream << emitter.c_str();
	fileOutStream.close();
	if (!fileOutStream)
	{
		returnError.Ok = false;
		returnError.Message.Append("Fail to close stream associated with the scene file in path: ").Append(scenePath.string().c_str()).Append(".");
	}
	return returnError;
}

}
