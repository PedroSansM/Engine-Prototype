#include "AnimationStateMachineManager.h"
#include "Parameter.h"
#include "ProgramContext.h"
#include "Log.h"
#include "Path.h"
#include "AnimationManager.h"

#include "yaml-cpp/yaml.h"

#include <fstream>
#include <ios>



namespace DEditor
{

static YAML::Node s_asmNode;				// Maps UUID -> Animation state machine path
static const char* s_asmDirectory{"animation state machine"};
static const char* s_asmsFileNameAndExtension{"asms.dasms"};
static const char* s_asmFileExtension{".dasm"};
static const char* s_thumbnailFileExtension{".dtasm"};
//
static const char* s_parametersKey{"Parameters"};
static const char* s_statesKey{"States"};
static const char* s_transitionsKey{"Transitions"};
static const char* s_parameterTypeKey{"ParameterType"};
static const char* s_parameterValueKey{"ParameterValue"};
static const char* s_stateAnimationKey{"Animation"};
static const char* s_statePositionKey{"StatePosition"};
static const char* s_isInitialStateKey{"IsInitialState"};
static const char* s_fromStateKey{"FromState"};
static const char* s_toStateKey{"ToState"};
static const char* s_conditionsKey{"Conditions"};
static const char* s_conditionParameterNameKey{"ParameterName"};
static const char* s_conditionNumericTypeKey{"NumericConditionType"};
static const char* s_conditionValueKey{"Value"};
//

AnimationStateMachineManager::AnimationStateMachineManager()
{
	pathType asmPath(GetAnimationStateMachinesPath() / s_asmsFileNameAndExtension);
	std::ofstream fstream(asmPath, std::ios_base::out | std::ios_base::app);
	DASSERT_E(fstream);
	fstream.close();
	DASSERT_E(fstream);
	s_asmNode = YAML::LoadFile(asmPath.string());
}

void AnimationStateMachineManager::CreateAnimationStateMachine(const stringType& name, const pathType& thumbnailDirectoryPath)
{
	pathType path(GetAnimationStateMachinesPath() / name);
	path += s_asmFileExtension;
	std::ifstream istream(path);
	if (istream)
	{
		Log::Get().TerminalLog("Animation state machine with name %s is already created.", name.data());
		Log::Get().ConsoleLog(LogLevel::Error, "Animation state machine with name %s is already created.", name.data());
		istream.close();
		return;
	}
	std::ofstream ostream(path);
	DASSERT_E(ostream);
	uuidType uuid;
	DCore::UUIDGenerator::Get().GenerateUUID(uuid);
	const stringType uuidString((stringType)uuid);
	s_asmNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(path).string();
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	{

		emitter << YAML::Key << s_parametersKey << YAML::Value << YAML::BeginMap << YAML::EndMap;
		emitter << YAML::Key << s_statesKey << YAML::Value << YAML::BeginMap << YAML::EndMap;
		emitter << YAML::Key << s_transitionsKey << YAML::Value << YAML::BeginSeq << YAML::EndSeq;
		emitter << YAML::EndMap;
	}
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	DASSERT_E(ostream);
	pathType thumbnailPath(thumbnailDirectoryPath / name);
	thumbnailPath += s_thumbnailFileExtension;
	GenerateThumbnail(thumbnailPath, uuidString);
	SaveAnimationStateMachineMap();
}

EditorAnimationStateMachine AnimationStateMachineManager::LoadAnimationStateMachine(
	const uuidType& uuid, 
	const stringType& name, 
	parameterInfoContainerType* outParameterInfos)
{
	using parameterType = DCore::ParameterType;
	using integerParameterConstRefType = DCore::integerParameterConstRefType;
	using floatParameterConstRefType = DCore::floatParameterConstRefType;
	using logicParameterConstRefType = DCore::logicParameterConstRefType;
	using triggerParameterConstRefType = DCore::triggerParameterConstRefType;
	using logicParameterType = DCore::LogicParameter;
	using triggerParameterType = DCore::TriggerParameter;
	using stateConstRefType = DCore::AnimationStateMachine::stateConstRefType;
	using transitionConstRefType = DCore::transitionConstRefType;
	using conditionConstRefType = DCore::conditionConstRefType;
	using createTransitionResult = DCore::AnimationStateMachine::CreateTransitionResult;
	using createConditionResult = DCore::AnimationStateMachine::CreateConditionResult;
	using numericConditionType = DCore::NumericConditionType;
	using coreAnimationRefType = DCore::AnimationRef;
	using dVec2 = DCore::DVec2;
	using nodeType = YAML::Node;
	using nodeConstItType = YAML::const_iterator;
	EditorAnimationStateMachine animationStateMachine(uuid);
	animationStateMachine.SetName(name);
	const stringType uuidString(uuid);
	DASSERT_E(s_asmNode[uuidString]);
	const nodeType asmNode(YAML::LoadFile((ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_asmNode[uuidString].as<stringType>()).string()));
	DASSERT_E(asmNode[s_parametersKey] && asmNode[s_statesKey] && asmNode[s_transitionsKey]);
	const nodeType parametersNode(asmNode[s_parametersKey]);
	const nodeType statesNode(asmNode[s_statesKey]);
	const nodeType transitionsNode(asmNode[s_transitionsKey]);
	for (nodeConstItType parameterIt(parametersNode.begin()); parameterIt != parametersNode.end(); parameterIt++)
	{
		const stringType parameterName(parameterIt->first.as<stringType>());
		const nodeType valueNode(parameterIt->second);
		DASSERT_E(valueNode[s_parameterTypeKey]);
		DASSERT_E(valueNode[s_parameterValueKey]);
		const parameterType parameterType(DCore::ParameterUtils::GetParameterType(valueNode[s_parameterTypeKey].as<stringType>()));
		switch (parameterType)
		{
		case parameterType::Integer:
		{
			integerParameterConstRefType parameter;
			animationStateMachine.CreateParameter<parameterType::Integer>(parameterName, parameter);
			animationStateMachine.SetParameterValue<parameterType::Integer>(parameter.GetIndex(), valueNode[s_parameterValueKey].as<int>());
			if (outParameterInfos != nullptr)
			{
				outParameterInfos->push_back({parameterType::Integer, parameter.GetIndex()});
			}
			break;
		}
		case parameterType::Float:
		{
			floatParameterConstRefType parameter;
			animationStateMachine.CreateParameter<parameterType::Float>(parameterName, parameter);
			animationStateMachine.SetParameterValue<parameterType::Float>(parameter.GetIndex(), valueNode[s_parameterValueKey].as<float>());
			if (outParameterInfos != nullptr)
			{
				outParameterInfos->push_back({parameterType::Float, parameter.GetIndex()});
			}
			break;
		}
		case parameterType::Logic:
		{
			logicParameterConstRefType parameter;
			animationStateMachine.CreateParameter<parameterType::Logic>(parameterName, parameter);
			animationStateMachine.SetParameterValue<parameterType::Logic>(parameter.GetIndex(), logicParameterType{valueNode[s_parameterValueKey].as<bool>()});
			if (outParameterInfos != nullptr)
			{
				outParameterInfos->push_back({parameterType::Logic, parameter.GetIndex()});
			}
			break;
		}
		case parameterType::Trigger:
		{
			triggerParameterConstRefType parameter;
			animationStateMachine.CreateParameter<parameterType::Trigger>(parameterName, parameter);
			animationStateMachine.SetParameterValue<parameterType::Trigger>(parameter.GetIndex(), triggerParameterType{valueNode[s_parameterValueKey].as<bool>()});
			if (outParameterInfos != nullptr)
			{
				outParameterInfos->push_back({parameterType::Trigger, parameter.GetIndex()});
			}
			break;
		}
		}
	}	
	for (nodeConstItType stateIt(statesNode.begin()); stateIt != statesNode.end(); stateIt++)
	{
		const stringType stateName(stateIt->first.as<stringType>());
		const nodeType valuesNode(stateIt->second);
		DASSERT_E(valuesNode[s_stateAnimationKey]);
		DASSERT_E(valuesNode[s_isInitialStateKey]);
		DASSERT_E(valuesNode[s_statePositionKey]);
		bool isInitialState(valuesNode[s_isInitialStateKey].as<bool>());
		const nodeType statePositionNode(valuesNode[s_statePositionKey]);
		DASSERT_E(statePositionNode[0] && statePositionNode[1]);
		const dVec2 statePosition(statePositionNode[0].as<float>(), statePositionNode[1].as<float>());
		stateConstRefType state;
		animationStateMachine.CreateState(stateName, statePosition, &state);
		if (isInitialState)
		{
			animationStateMachine.SetInitialStateIndex(state.GetIndex());
		}
		const stringType animationUUIDString(valuesNode[s_stateAnimationKey].as<stringType>());
		if (!animationUUIDString.empty())
		{
			const uuidType animationUUID(animationUUIDString);
			coreAnimationRefType animation(AnimationManager::Get().LoadCoreAnimation(animationUUID));
			// TODO. Lock AnimationAssetManager for reading.
			DASSERT_E(animation.IsValid());
			animationStateMachine.SetStateAnimation(state.GetIndex(), animation);
		}
	}
	for (nodeConstItType transitionIt(transitionsNode.begin()); transitionIt != transitionsNode.end(); transitionIt++)
	{
		const nodeType transitionNode(*transitionIt);
		DASSERT_E(transitionNode[s_fromStateKey] && transitionNode[s_toStateKey] && transitionNode[s_conditionsKey]);
		const stringType fromStateName(transitionNode[s_fromStateKey].as<stringType>());
		const stringType toStateName(transitionNode[s_toStateKey].as<stringType>());
		stateConstRefType fromState(animationStateMachine.TryGetStateWithName(fromStateName));
		stateConstRefType toState(animationStateMachine.TryGetStateWithName(toStateName));
		DASSERT_E(fromState.IsValid() && toState.IsValid());
		transitionConstRefType transition;
		createTransitionResult transitionResult(animationStateMachine.CreateTransition(fromState.GetIndex(), toState.GetIndex(), &transition));
		DASSERT_E(transitionResult == createTransitionResult::Ok);
		const nodeType conditionsNode(transitionNode[s_conditionsKey]);
		for (nodeConstItType conditionIt(conditionsNode.begin()); conditionIt != conditionsNode.end(); conditionIt++)
		{
			conditionConstRefType condition;
			createConditionResult conditionResult(animationStateMachine.CreateCondition(fromState.GetIndex(), transition.GetIndex(), &condition));
			DASSERT_E(conditionResult == createConditionResult::Ok);
			const nodeType conditionNode(*conditionIt);
			DASSERT_E(conditionNode[s_conditionParameterNameKey]);
			const stringType conditionParameterName(conditionNode[s_conditionParameterNameKey].as<stringType>());
			DASSERT_E(parametersNode[conditionParameterName.c_str()]);
			const nodeType parameterNode(parametersNode[conditionParameterName.c_str()]);
			DASSERT_E(parameterNode[s_parameterTypeKey]);
			const parameterType conditionParameterType(DCore::ParameterUtils::GetParameterType(parameterNode[s_parameterTypeKey].as<stringType>()));
			switch (conditionParameterType)
			{
			case parameterType::Integer:
			{
				DASSERT_E(conditionNode[s_conditionNumericTypeKey] && conditionNode[s_conditionValueKey]);
				const numericConditionType numericCondition(DCore::Condition::GetNumericCondition(conditionNode[s_conditionNumericTypeKey].as<stringType>()));
				integerParameterConstRefType parameter(animationStateMachine.TryGetParameterWithName<parameterType::Integer>(conditionParameterName));
				DASSERT_E(parameter.IsValid());
				animationStateMachine.SetConditionParameter(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), parameterType::Integer, parameter.GetIndex());
				animationStateMachine.SetNumericCondition(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), numericCondition);
				animationStateMachine.SetValueOfCondition<parameterType::Integer>(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), conditionNode[s_conditionValueKey].as<int>());
				break;
			}
			case parameterType::Float:
			{
				DASSERT_E(conditionNode[s_conditionNumericTypeKey] && conditionNode[s_conditionValueKey]);
				const numericConditionType numericCondition(DCore::Condition::GetNumericCondition(conditionNode[s_conditionNumericTypeKey].as<stringType>()));
				floatParameterConstRefType parameter(animationStateMachine.TryGetParameterWithName<parameterType::Float>(conditionParameterName));
				DASSERT_E(parameter.IsValid());
				animationStateMachine.SetConditionParameter(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), parameterType::Float, parameter.GetIndex());
				animationStateMachine.SetNumericCondition(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), numericCondition);
				animationStateMachine.SetValueOfCondition<parameterType::Float>(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), conditionNode[s_conditionValueKey].as<float>());
				break;
			}
			case parameterType::Logic:
			{
				DASSERT_E(conditionNode[s_conditionValueKey]);
				logicParameterConstRefType parameter(animationStateMachine.TryGetParameterWithName<parameterType::Logic>(conditionParameterName));
				DASSERT_E(parameter.IsValid());
				animationStateMachine.SetConditionParameter(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), parameterType::Logic, parameter.GetIndex());
				animationStateMachine.SetValueOfCondition<parameterType::Logic>(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), logicParameterType{conditionNode[s_conditionValueKey].as<bool>()});
				break;
			}
			case parameterType::Trigger:
			{
				triggerParameterConstRefType parameter(animationStateMachine.TryGetParameterWithName<parameterType::Trigger>(conditionParameterName));
				DASSERT_E(parameter.IsValid());
				animationStateMachine.SetConditionParameter(fromState.GetIndex(), transition.GetIndex(), condition.GetIndex(), parameterType::Trigger, parameter.GetIndex());
				break;
			}
			}
		}
	}
	return animationStateMachine;
}

void AnimationStateMachineManager::SaveChanges(const animationStateMachineType& animationStateMachine, const parameterInfoContainerType& parameterInfos)
{
	using uuidType = DCore::UUIDType;
	using parameterType = DCore::ParameterType;
	using integerParameterConstRefType = DCore::integerParameterConstRefType;
	using floatParameterConstRefType = DCore::floatParameterConstRefType;
	using logicParameterConstRefType = DCore::logicParameterConstRefType;
	using triggerParameterConstRefType = DCore::triggerParameterConstRefType;
	using stateConstRefType = DCore::AnimationStateMachine::stateConstRefType;
	using transitionConstRefType = DCore::transitionConstRefType;
	using dVec2 = DCore::DVec2;
	using conditionConstRefType = DCore::conditionConstRefType;
	using conditionType = DCore::Condition;
	const uuidType& uuid(animationStateMachine.GetUUID());
	const stringType uuidString(uuid);
	DASSERT_E(s_asmNode[uuidString]);
	const pathType path(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_asmNode[uuidString].as<stringType>());
	YAML::Emitter emitter;
	const auto serializeParameter
	(
		 [&](auto parameter) -> void
		 {
			emitter << YAML::Key << parameter->GetName();
			emitter << YAML::Value << YAML::BeginMap;
			{
				emitter << YAML::Key << s_parameterTypeKey << YAML::Value << DCore::ParameterUtils::GetParameterTypeName(parameter->GetType());
				emitter << YAML::Key << s_parameterValueKey << YAML::Value << parameter->GetLiteralValue();
				emitter << YAML::EndMap;
			}
		 }
	);
	emitter << YAML::BeginMap;
	{
		emitter << YAML::Key << s_parametersKey << YAML::Value << YAML::BeginMap;
		{
			for (const parameterInfoType& parameterInfo : parameterInfos)
			{
				const size_t parameterIndex(parameterInfo.Index);
				switch (parameterInfo.Type) 
				{
				case parameterType::Integer:
				{
					integerParameterConstRefType parameter(animationStateMachine.TryGetParameterAtIndex<parameterType::Integer>(parameterIndex));
					DASSERT_E(parameter.IsValid());
					serializeParameter(parameter);
					break;
				}
				case parameterType::Float:
				{
					floatParameterConstRefType parameter(animationStateMachine.TryGetParameterAtIndex<parameterType::Float>(parameterIndex));
					DASSERT_E(parameter.IsValid());
					serializeParameter(parameter);
					break;
				}
				case parameterType::Logic:
				{
					logicParameterConstRefType parameter(animationStateMachine.TryGetParameterAtIndex<parameterType::Logic>(parameterIndex));
					DASSERT_E(parameter.IsValid());
					serializeParameter(parameter);
					break;
				}
				case parameterType::Trigger:
				{
					triggerParameterConstRefType parameter(animationStateMachine.TryGetParameterAtIndex<parameterType::Trigger>(parameterIndex));
					DASSERT_E(parameter.IsValid());
					serializeParameter(parameter);
					break;
				}
				}
			}
			emitter << YAML::EndMap;
		}
		emitter << YAML::Key << s_statesKey << YAML::Value << YAML::BeginMap;
		{
			animationStateMachine.IterateOnStates
			(
				[&](stateConstRefType state) -> bool
				{
					emitter << YAML::Key << state->GetName();
					emitter << YAML::Value << YAML::BeginMap;
					{
						emitter << YAML::Key << s_stateAnimationKey << YAML::Value << (state->GetAnimation().IsValid() ? (stringType)state->GetAnimation().GetUUID() : "");
						emitter << YAML::Key << s_isInitialStateKey << YAML::Value << (state.GetIndex() == animationStateMachine.GetInitialStateIndex());
						emitter << YAML::Key << s_statePositionKey << YAML::Value << YAML::Flow << YAML::BeginSeq;
						{
							const dVec2& statePosition(animationStateMachine.GetPositionOfStateAtIndex(state.GetIndex()));
							emitter << statePosition.x << statePosition.y;
							emitter << YAML::EndSeq;
						}
						emitter << YAML::EndMap;
					}
					return false;
				}
			);
			emitter << YAML::EndMap;
		}
		emitter << YAML::Key << s_transitionsKey << YAML::Value << YAML::BeginSeq;
		{
			animationStateMachine.IterateOnStates
			(
				[&](stateConstRefType state) -> bool
				{
					state->IterateOnTransitions
					(
						[&](transitionConstRefType transition) -> bool
						{
							emitter << YAML::BeginMap;
							{
								emitter << YAML::Key << s_fromStateKey << YAML::Value << state->GetName();
								stateConstRefType toState(animationStateMachine.TryGetStateAtIndex(transition->GetToStateIndex()));
								DASSERT_E(toState.IsValid());
								emitter << YAML::Key << s_toStateKey << YAML::Value << toState->GetName();
								emitter << YAML::Key << s_conditionsKey << YAML::Value << YAML::BeginSeq;
								{
									transition->IterateOnConditions
									(
										[&](conditionConstRefType condition) -> bool
										{
											emitter << YAML::BeginMap;
											{
												emitter << YAML::Key << s_conditionParameterNameKey;
												const parameterType conditionParameterType(condition->GetParameterType());
												switch (conditionParameterType)
												{
												case parameterType::Integer:
													emitter << YAML::Value << condition->GetParameter<parameterType::Integer>()->GetName().c_str();
													emitter << YAML::Key << s_conditionNumericTypeKey << YAML::Value << conditionType::GetNumericConditionTypeName(condition->GetNumericCondition());
													emitter << YAML::Key << s_conditionValueKey << YAML::Value << condition->GetValue<parameterType::Integer>();
													break;
												case parameterType::Float:
													emitter << YAML::Value << condition->GetParameter<parameterType::Float>()->GetName().c_str();
													emitter << YAML::Key << s_conditionNumericTypeKey << YAML::Value << conditionType::GetNumericConditionTypeName(condition->GetNumericCondition());
													emitter << YAML::Key << s_conditionValueKey << YAML::Value << condition->GetValue<parameterType::Float>();
													break;
												case parameterType::Logic:
													emitter << YAML::Value << condition->GetParameter<parameterType::Logic>()->GetName().c_str();
													emitter << YAML::Key << s_conditionValueKey << YAML::Value << condition->GetValue<parameterType::Logic>().Value;
													break;
												case parameterType::Trigger:
													emitter << YAML::Value << condition->GetParameter<parameterType::Trigger>()->GetName().c_str();
													break;
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
							return false;
						}
					);
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

AnimationStateMachineManager::stringType AnimationStateMachineManager::GetAnimationStateMachineName(const uuidType& uuid) 
{
	const stringType uuidString(uuid);
	DASSERT_E(s_asmNode[uuidString]);
	return pathType(s_asmNode[uuidString].as<stringType>()).stem().string();
}

void AnimationStateMachineManager::RemoveAnimationReferences(const uuidType& animationUUID)
{
	for (YAML::const_iterator animationStateMachinesIt(s_asmNode.begin()); animationStateMachinesIt != s_asmNode.end(); animationStateMachinesIt++)
	{
		const pathType asmPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / animationStateMachinesIt->second.as<stringType>());
		YAML::Node asmNode(YAML::LoadFile(asmPath.string()));
		DASSERT_E(asmNode[s_statesKey]);
		YAML::Node statesNode(asmNode[s_statesKey]);
		for (YAML::const_iterator statesIt(statesNode.begin()); statesIt != statesNode.end(); statesIt++)
		{
			YAML::Node stateInfoNode(statesIt->second);
			DASSERT_E(stateInfoNode[s_stateAnimationKey]);
			const uuidType stateAnimationUUID(stateInfoNode[s_stateAnimationKey].as<stringType>());
			if (stateAnimationUUID == animationUUID)
			{
				stateInfoNode[s_stateAnimationKey] = "";
			}
			statesNode[statesIt->first] = stateInfoNode;
		}
		asmNode[s_statesKey] = statesNode;
		YAML::Emitter emitter;
		emitter << asmNode;
		DASSERT_E(emitter.good());
		std::ofstream ostream(asmPath);
		DASSERT_E(ostream);
		ostream << emitter.c_str();
		ostream.close();
		DASSERT_E(ostream);
	}
}

void AnimationStateMachineManager::DeleteAnimationStateMachine(const uuidType& asmUUID)
{
	const stringType uuidString(asmUUID);
	DASSERT_E(s_asmNode[uuidString]);
	const pathType asmPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_asmNode[uuidString].as<stringType>());
	std::filesystem::remove(asmPath);
	YAML::Node newAsmNode;
	for (YAML::const_iterator it(s_asmNode.begin()); it != s_asmNode.end(); it++)
	{
		const uuidType uuid(it->first.as<stringType>());
		if (uuid == asmUUID)
		{
			continue;
		}
		newAsmNode[it->first] = it->second;
	}
	s_asmNode = newAsmNode;
	YAML::Emitter emitter;
	emitter << s_asmNode;
	DASSERT_E(emitter.good());
	std::ofstream ostream(GetAnimationStateMachinesPath() / s_asmsFileNameAndExtension);
	DASSERT_E(ostream);
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
	DCore::AssetManager::Get().RemoveAllAnimationStateMachineInstancesWithUUID(asmUUID);
}

bool AnimationStateMachineManager::RenameAnimationStateMachine(const uuidType& uuid, const stringType& newName)
{
	for (YAML::const_iterator it(s_asmNode.begin()); it != s_asmNode.end(); it++)
	{
		const pathType asmPath(it->second.as<stringType>());
		if (asmPath.stem() == newName)
		{
			Log::Get().TerminalLog("There is already a animation state machine with name with name %s.", newName.c_str());
			Log::Get().ConsoleLog(LogLevel::Error, "There is already a animation state machine with name %s.", newName.c_str());
			return false;
		}
	}
	const stringType uuidString(uuid);
	DASSERT_E(s_asmNode[uuidString]);
	const pathType oldPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_asmNode[uuidString].as<stringType>());
	pathType newPath(oldPath);
	newPath.replace_filename(newName + ".dasm");
	s_asmNode[uuidString] = Path::Get().MakePathRelativeToAssetsDirectory(newPath).string();
	SaveAnimationStateMachineMap();
	std::filesystem::rename(oldPath, newPath);
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationStateMachineAssetManager*>(&DCore::AssetManager::Get()));
	DCore::AssetManager::Get().IterateOnAnimationStateMachines
	(
		[&](auto animationStateMachine) -> bool
		{
			if (animationStateMachine->GetUUID() == uuid)
			{
				animationStateMachine->GetAsset().SetName(newName);
			}
			return false;
		}
	);
	return true;
}

AnimationStateMachineManager::pathType AnimationStateMachineManager::GetAnimationStateMachinesPath() const
{
	pathType asmPath(ProgramContext::Get().GetProjectAssetsDirectoryPath() / s_asmDirectory);
	return asmPath;
}

void AnimationStateMachineManager::GenerateThumbnail(const pathType& path, const stringType& uuidString)
{
	std::ofstream ostream(path);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << YAML::BeginMap;
	{
		emitter << YAML::Key << "UUID" << YAML::Value << uuidString;
		emitter << YAML::EndMap;
	}
	DASSERT_E(emitter.good());
	ostream << emitter.c_str();
	DASSERT_E(ostream);
	ostream.close();
	DASSERT_E(ostream);
}

void AnimationStateMachineManager::SaveAnimationStateMachineMap()
{
	pathType path(GetAnimationStateMachinesPath() / s_asmsFileNameAndExtension);
	std::ofstream ostream(path);
	DASSERT_E(ostream);
	YAML::Emitter emitter;
	emitter << s_asmNode;
	ostream << emitter.c_str();
	ostream.close();
	DASSERT_E(ostream);
}

}
