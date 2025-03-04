#include "InspectorPanel.h"
#include "Log.h"
#include "TextureManager.h"
#include "ProgramContext.h"
#include "EditorAnimationStateMachine.h"
#include "EditorAnimation.h"
#include "AnimationManager.h"
#include "AnimationPayload.h"
#include "AnimationStateMachineManager.h"
#include "AttributeRef.h"
#include "AnimationStateMachinePayload.h"
#include "PhysicsMaterialPayload.h"
#include "PhysicsMaterialManager.h"
#include "TypeNames.h"

#include "DommusCore.h"

#include "fmod_common.h"
#include "imgui.h"
#include "glm/gtc/integer.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <type_traits>
#include <inttypes.h>



namespace DEditor
{

InspectorPanel::InspectorPanel()
	:
	m_isOpened(true),
	m_whiteMap(TextureManager::Get().LoadRawTexture2D(ProgramContext::Get().GetEditorAssetsDirectoryPath() / "texture" / "WhiteMap.png"))
{}

void InspectorPanel::DisplayEntity(entityRefType entityRef)
{
	m_renderFunction = &InspectorPanel::RenderEntity;
	m_context.SetEntity(entityRef);
	(this->*m_renderFunction)(true);
}

void InspectorPanel::DisplayTransition(size_t transitionIndex, size_t fromStateIndex, asmPanelRefType asmPanel)
{
	m_renderFunction = &InspectorPanel::RenderTransition;
	m_context.SetTranstion(transitionIndex, fromStateIndex, asmPanel);
	(this->*m_renderFunction)(true);
}

void InspectorPanel::DisplayState(stateConstRefType state, animationStateMachineType& animationStateMachine)
{
	m_renderFunction = &InspectorPanel::RenderState;
	m_context.SetState(state, animationStateMachine);
	(this->*m_renderFunction)(true);
}

void InspectorPanel::Open()
{
	m_isOpened = true;
}

void InspectorPanel::Render()
{
	if (!m_isOpened)
	{
		return;
	}
	ImGuiWindowFlags windowFlags(0);
	if (!ImGui::Begin("Inspector", &m_isOpened, windowFlags))
	{
		ImGui::End();
		return;
	}
	if (m_renderFunction != nullptr)
	{
		if (m_renderFunction == &InspectorPanel::RenderEntity)
		{
			DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
			(this->*m_renderFunction)(false);
		}
		else
		{
			(this->*m_renderFunction)(false);
		}
	}
	ImGui::End();
}

void InspectorPanel::RenderEntity(bool toSetup)
{
	static DCore::DString entityNameBuffer;
	if (!m_context.Content.Entity.IsValid())
	{
		return;
	}
	if (toSetup)
	{
		m_context.Content.Entity.GetName(entityNameBuffer);
		DCore::ComponentRef<DCore::TransformComponent> transformComponent(m_context.Content.Entity.GetComponents<DCore::TransformComponent>());
		DASSERT_E(transformComponent.IsValid());
		return;
	}
	static bool itemActive(false);
	static ImGuiInputTextFlags inputTextFlags(ImGuiInputTextFlags_EnterReturnsTrue);
	bool nameProvided(ImGui::InputText("##EntityName", entityNameBuffer.Data(), sizeof(stringType), inputTextFlags));
	if (nameProvided)
	{
		if (entityNameBuffer.Empty())
		{
			Log::Get().ConsoleLog(LogLevel::Warning, "%s", "Entity name cannot be empty.");
			m_context.Content.Entity.GetName(entityNameBuffer);
		}
		m_context.Content.Entity.SetName(entityNameBuffer.Data());
	}
	if (!ImGui::IsItemActive())
	{
		if (itemActive)
		{
			m_context.Content.Entity.GetName(entityNameBuffer);
			itemActive = false;
		}
	}
	else
	{
		itemActive = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add"))
	{
		ImGui::OpenPopup("Add Component Popup");
	}
	if (ImGui::BeginPopup("Add Component Popup"))
	{
		const std::vector<DCore::ComponentForm>& componentForms(DCore::ComponentForms::Get().GetComponentForms());
		for (const DCore::ComponentForm& componentForm : componentForms)
		{
			const DCore::ComponentIdType componentId(componentForm.Id);
			if (
				componentId == DCore::ComponentId::GetId<DCore::NameComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::TransformComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::UUIDComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::ChildrenComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::ChildComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::RootComponent>())
			{
				continue;
			}
			if (m_context.Content.Entity.HaveComponent(componentId))
			{
				continue;
			}
			if (ImGui::Button(componentForm.Name.data()))
			{
				m_context.Content.Entity.AddComponents
				(
					&componentId, &componentForm.TotalSize, 1,
					[&](DCore::ComponentIdType componentId, void* componentAddress) -> void
					{
						componentForm.PlacementNewConstructor(componentAddress, componentForm.DefaultArgs);
					}
				);
			}
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform"))
	{
		DCore::ComponentRef<DCore::TransformComponent> transformComponent(m_context.Content.Entity.GetComponents<DCore::TransformComponent>());
		DASSERT_E(transformComponent.IsValid());
		ImGuiTreeNodeFlags treeNodeFlags(ImGuiTreeNodeFlags_SpanAvailWidth);
		DCore::DVec3 translation(transformComponent.GetTranslation());
		DCore::DFloat rotation(transformComponent.GetRotation());
		DCore::DVec2 scale(transformComponent.GetScale());
		if (DrawVector3Field("Translation", "X", "Y", "Z", translation))
		{
			transformComponent.SetTranslation(translation);
			transformComponent.OnAttributeChange(DCore::TransformComponent::a_translation, &translation, DCore::AttributeType::Vector3);
		}
		if (DrawFloatField("Rotation", rotation))
		{
			transformComponent.SetRotation(rotation);
			transformComponent.OnAttributeChange(DCore::TransformComponent::a_rotation, &rotation, DCore::AttributeType::Float);
		}
		if (DrawVector2Field("Scale", "X", "Y", scale))
		{
			transformComponent.SetScale(scale);
			transformComponent.OnAttributeChange(DCore::TransformComponent::a_scale, &scale, DCore::AttributeType::Vector2);
		}
	}
	m_context.Content.Entity.IterateOnComponents
	(
		[&](DCore::ComponentIdType componentId, void*) -> bool
		{
			if 
			(
				componentId == DCore::ComponentId::GetId<DCore::NameComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::TransformComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::UUIDComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::ChildrenComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::ChildComponent>() ||
				componentId == DCore::ComponentId::GetId<DCore::RootComponent>()
			)
			{
				return false;
			}
			const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
			if (!ImGui::CollapsingHeader(componentForm.Name.data()))
			{
				return false;
			}
			if (ImGui::BeginPopupContextItem())
			{
				bool componentRemoved(false);
				if (ImGui::Selectable("Remove Component"))
				{
					m_context.Content.Entity.RemoveComponents(&componentId, 1);
					componentRemoved = true;
				}
				ImGui::EndPopup();
				if (componentRemoved)
				{
					return false;
				}
			}
			DCore::ComponentRef<DCore::Component> component(m_context.Content.Entity.GetComponent(componentId));
			DASSERT_E(component.IsValid());
			for (const serializedAttributeType& attribute : componentForm.SerializedAttributes)
			{
				const DCore::AttributeName& attributeName(attribute.GetAttributeName());
				DCore::AttributeType attributeType(attribute.GetAttributeType());
				DCore::AttributeIdType attributeId(attribute.GetAttributeId());
				switch (attributeType)
				{
				case DCore::AttributeType::Integer:
				{
					DCore::DInt value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DInt));
					if (DrawIntegerField(attributeName.GetName().c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::UInteger:
				{
					DCore::DUInt value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DUInt));
					if (DrawUIntegerField(attributeName.GetName().c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::Logic:
				{
					DCore::DLogic value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DLogic));
					if (DrawLogicField(attributeName.GetName().c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::Float:
				{
					DCore::DFloat value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DFloat));
					if (DrawFloatField(attributeName.GetName().c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::Vector2:
				{
					DCore::DVec2 value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec2));
					if (DrawVector2Field(attributeName.GetName().c_str(), attributeName.GetComponentAtIndex(0).c_str(), attributeName.GetComponentAtIndex(1).c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::UIVector2:
				{
					DCore::DVec2 value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec2));
					if (DrawUIVector2Field(attributeName.GetName().c_str(), attributeName.GetComponentAtIndex(0).c_str(), attributeName.GetComponentAtIndex(1).c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::Vector3:
				{
					DCore::DVec3 value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec3));
					if (DrawVector3Field(attributeName.GetName().c_str(), attributeName.GetComponentAtIndex(0).c_str(), attributeName.GetComponentAtIndex(1).c_str(), attributeName.GetComponentAtIndex(2).c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::SpriteMaterial:
				{
					DCore::SpriteMaterialRef spriteMaterial;
					component.GetAttributePtr(attributeId, &spriteMaterial, sizeof(DCore::SpriteMaterialRef));
					if (DrawSpriteMaterialField(attributeName.GetName().c_str(), spriteMaterial))
					{
						component.OnAttributeChange(attributeId, &spriteMaterial, attributeType);
					}
					break;
				}
				case DCore::AttributeType::TaggedList:
				{
					char tagName[256];
					std::memset(tagName, '\0', 256);
					size_t currentTagIndex;
					component.GetAttributePtr(attributeId, &currentTagIndex, sizeof(size_t));
					if (DrawTaggedList(attributeName.GetName().c_str(), currentTagIndex, attributeName, tagName))
					{
						component.OnAttributeChange(attributeId, tagName, DCore::AttributeType::TaggedList);
					}
					break;
				}
				case DCore::AttributeType::Color:
				{
					DCore::DVec4 value;
					component.GetAttributePtr(attributeId, &value, sizeof(DCore::DVec4));
					if (DrawColorPickerField(attributeName.GetName().c_str(), value))
					{
						component.OnAttributeChange(attributeId, &value, attributeType);
					}
					break;
				}
				case DCore::AttributeType::AnimationStateMachine:
				{
					coreAnimationStateMachineRefType animationStateMachine;
					coreAnimationStateMachineRefType newAnimationStateMachine;
					component.GetAttributePtr(attributeId, &animationStateMachine, sizeof(DCore::AnimationStateMachineRef));
					if (DrawAnimationStateMachineField(attributeName.GetName().c_str(), animationStateMachine, newAnimationStateMachine))
					{
						component.OnAttributeChange(attributeId, &newAnimationStateMachine, DCore::AttributeType::AnimationStateMachine);
					}
					break;
				}
				case DCore::AttributeType::PhysicsBodyType:
				{
					bodyType newBodyType;
					component.GetAttributePtr(attributeId, &newBodyType, sizeof(bodyType));
					if (DrawPhysicsBodyType(attributeName.GetName().c_str(), newBodyType))
					{
						component.OnAttributeChange(attributeId, &newBodyType, DCore::AttributeType::PhysicsBodyType);
					}
					break;
				}
				case DCore::AttributeType::PhysicsMaterial:
				{
					physicsMaterialRefType currentPhysicsMaterial;
					physicsMaterialRefType newPhysicsMaterial;
					component.GetAttributePtr(attributeId, &currentPhysicsMaterial, sizeof(physicsMaterialRefType));
					if (DrawPhysicsMaterialField(attributeName.GetName().c_str(), currentPhysicsMaterial, newPhysicsMaterial))
					{
						DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::PhysicsMaterialAssetManager*>(&DCore::AssetManager::Get()));
						component.OnAttributeChange(attributeId, &newPhysicsMaterial, DCore::AttributeType::PhysicsMaterial);
					}
					break;
				}
				case DCore::AttributeType::PhysicsLayer:
				{
					physicsLayerType physicsLayer;
					component.GetAttributePtr(attributeId, &physicsLayer, sizeof(physicsLayerType));
					if (DrawPhysicsLayerField(attributeName.GetName().c_str(), physicsLayer))
					{
						component.OnAttributeChange(attributeId, &physicsLayer, DCore::AttributeType::PhysicsLayer);
					}
					break;
				}
				case DCore::AttributeType::PhysicsLayers:
				{
					physicsLayerType physicsLayers;
					component.GetAttributePtr(attributeId, &physicsLayers, sizeof(physicsLayerType));
					if (DrawPhysicsLayersField(attributeName.GetName().c_str(), physicsLayers))
					{
						component.OnAttributeChange(attributeId, &physicsLayers, DCore::AttributeType::PhysicsLayers);
					}
					break;
				}
				case DCore::AttributeType::SoundEventInstance:
				{
					soundEventInstanceType soundEventInstance;
					component.GetAttributePtr(attributeId, &soundEventInstance, sizeof(soundEventInstanceType));
					if (DrawSoundEventInstanceField(attributeName.GetName().c_str(), soundEventInstance))
					{
						component.OnAttributeChange(attributeId, &soundEventInstance, DCore::AttributeType::SoundEventInstance);
					}
					soundEventInstance.Invalidate();
					break;
				}
				default:
					break;
				}
			}
			return false;
		}
	);
}

void InspectorPanel::RenderTransition(bool toSetup)
{
	asmPanelRefType asmPanel(m_context.Content.AnimationStateMachineTransitionContent.AsmPanel);
	transitionConstRefType transition(
		asmPanel->GetEditorAnimationStateMachine().TryGetTransitionAtIndex(
			m_context.Content.AnimationStateMachineTransitionContent.FromStateIndex, 
			m_context.Content.AnimationStateMachineTransitionContent.TransitionIndex));
	DASSERT_E(transition.IsValid());
	animationStateMachineType& animationStateMachine(m_context.Content.AnimationStateMachineTransitionContent.AsmPanel->GetEditorAnimationStateMachine());
	using stateConstRefType = EditorAnimationStateMachine::stateConstRefType;
	using createConditionResult = EditorAnimationStateMachine::createConditionResult;
	using parameterType = DCore::ParameterType;
	using numericConditionType = DCore::NumericConditionType;
	using logicParameterType = DCore::LogicParameter;
	if (!transition.IsValid())
	{
		return;
	}
	stateConstRefType fromState(animationStateMachine.TryGetStateAtIndex(transition->GetFromStateIndex()));
	stateConstRefType toState(animationStateMachine.TryGetStateAtIndex(transition->GetToStateIndex()));
	DASSERT_E(fromState.IsValid() && toState.IsValid());
	ImGui::Text("Transition");
	ImGui::Indent();
	ImGui::Text("%s -> %s", fromState->GetName().c_str(), toState->GetName().c_str());
	ImGui::Unindent();
	if (ImGui::TreeNodeEx("Conditions"))
	{
		if (ImGui::Button("Add"))
		{
			switch (animationStateMachine.CreateCondition(fromState.GetIndex(), transition.GetIndex())) 
			{
			case createConditionResult::NoParameterCreated:
				Log::Get().ConsoleLog(LogLevel::Error, "%s", "No parameter in animation state machine.");
				break;
			default:
				break;
			}	
		}
		const ImVec2 regionAvailable(ImGui::GetContentRegionAvail());
		const auto drawNumericCondition
		(
			[&](conditionConstRefType condition) -> void
			{
				static const char* numericConditions[3]{">", "=", "<"};
				static const numericConditionType numericConditionsTypes[3]{numericConditionType::Bigger, numericConditionType::Equals, numericConditionType::Smaller};
				const size_t conditionIndex(condition.GetIndex());
				size_t numericConditionIndex(0);
				std::stringstream labelStream;
				labelStream << "##N" << conditionIndex;
				switch (condition->GetNumericCondition())
				{
				case numericConditionType::Bigger:
					numericConditionIndex = 0;
					break;
				case numericConditionType::Equals:
					numericConditionIndex = 1;
					break;
				case numericConditionType::Smaller:
					numericConditionIndex = 2;
					break;
				}
				ImGui::SetNextItemWidth(regionAvailable.x * comboWidthFactor);
				if (ImGui::BeginCombo(labelStream.str().c_str(), numericConditions[numericConditionIndex]))
				{
					for (size_t index(0); index < 3; index++)
					{
						if (ImGui::Selectable(numericConditions[index]))
						{
							animationStateMachine.SetNumericCondition(fromState.GetIndex(), transition.GetIndex(), conditionIndex, numericConditionsTypes[index]);
						}
					}
					ImGui::EndCombo();
				}
			}
		);
		animationStateMachine.IterateOnConditions
		(
			fromState.GetIndex(), 
			transition.GetIndex(), 
			[&](conditionConstRefType localCondition) -> bool
			{
				const size_t conditionIndex(localCondition.GetIndex());
				const size_t fromStateIndex(fromState.GetIndex());
				const size_t transitionIndex(transition.GetIndex());
				std::stringstream valueLabelStream;
				valueLabelStream << "##V" << localCondition.GetIndex();
				switch (localCondition->GetParameterType())
				{
				case parameterType::Integer:
				{
					integerParameterConstRefType currentParameter(localCondition->GetParameter<parameterType::Integer>());
					if (DrawParameterSelection(localCondition, currentParameter, animationStateMachine, fromState, toState, transition))
					{
						return true;
					}
					ImGui::SameLine();
					drawNumericCondition(localCondition);
					ImGui::SameLine();
					int value(localCondition->GetValue<parameterType::Integer>());
					ImGui::SetNextItemWidth(regionAvailable.x * comboWidthFactor);
					if (ImGui::DragInt(valueLabelStream.str().c_str(), &value))
					{
						animationStateMachine.SetValueOfCondition<parameterType::Integer>(fromStateIndex, transitionIndex, conditionIndex, value);
					}
					break;
				}
				case parameterType::Float:
				{
					floatParameterConstRefType currentParameter(localCondition->GetParameter<parameterType::Float>());
					if (DrawParameterSelection(localCondition, currentParameter, animationStateMachine, fromState, toState, transition))
					{
						return true;
					}
					ImGui::SameLine();
					drawNumericCondition(localCondition);
					ImGui::SameLine();
					float value(localCondition->GetValue<parameterType::Float>());
					ImGui::SetNextItemWidth(regionAvailable.x * comboWidthFactor);
					if (ImGui::DragFloat(valueLabelStream.str().c_str(), &value, 0.01f))
					{
						animationStateMachine.SetValueOfCondition<parameterType::Float>(fromStateIndex, transitionIndex, conditionIndex, value);
					}
					break;
				}
				case parameterType::Logic:
				{
					static const char* logicValues[2]{"true", "false"};
					static bool literalLogiclValues[2]{true, false};
					logicParameterConstRefType currentParameter(localCondition->GetParameter<parameterType::Logic>());
					if (DrawParameterSelection(localCondition, currentParameter, animationStateMachine, fromState, toState, transition))
					{
						return true;
					}
					ImGui::SameLine();
					logicParameterType value(localCondition->GetValue<parameterType::Logic>());
					const size_t currentValueIndex(value.Value ? 0 : 1);
					ImGui::SetNextItemWidth(regionAvailable.x * comboWidthFactor);
					if (ImGui::BeginCombo(valueLabelStream.str().c_str(), logicValues[currentValueIndex]))
					{
						for (size_t index(0); index < 2; index++)
						{
							if (ImGui::Selectable(logicValues[index]))
							{
								animationStateMachine.SetValueOfCondition<parameterType::Logic>(fromStateIndex, transitionIndex, conditionIndex, logicParameterType{literalLogiclValues[index]});
							}
						}
						ImGui::EndCombo();
					}
					break;
				}
				case parameterType::Trigger:
				{
					triggerParameterConstRefType currentParameter(localCondition->GetParameter<parameterType::Trigger>());
					if (DrawParameterSelection(localCondition, currentParameter, animationStateMachine, fromState, toState, transition))
					{
						return true;
					}
					break;
				}
				}
				return false;
			}
		);
		ImGui::TreePop();
	}
}

void InspectorPanel::RenderState(bool toSetup)
{
	using uuidType = DCore::UUIDType;
	using animationRefType = DCore::AnimationRef;
	stateConstRefType state(m_context.Content.AnimationStateMachineStateContent.State);
	animationStateMachineType& animationStateMachine(m_context.Content.AnimationStateMachineStateContent.AnimationStateMachine);
	if (!state.IsValid())
	{
		return;
	}
	ImGui::Text("%s", state->GetName().c_str());
	const animationRefType animation(state->GetAnimation());
	ImGui::Text("%s", "Animation");
	ImGui::Indent();
	if (!animation.IsValid())
	{
		ImGui::Text("No animation defined");
	}
	else
	{
		ImGui::Text("%s", animation.GetName().c_str());
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload(ImGui::AcceptDragDropPayload("ANIMATION_PAYLOAD"));
		if (payload != nullptr)
		{
			const AnimationPayload& animationPayload(*static_cast<AnimationPayload*>(payload->Data));
			animationStateMachine.SetStateAnimation(state.GetIndex(), AnimationManager::Get().LoadCoreAnimation(animationPayload.UUID));
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::Unindent();
#undef ASM
}

bool InspectorPanel::DrawIntegerField(const char* fieldName, DCore::DInt& out)
{
	return false;
}

bool InspectorPanel::DrawUIntegerField(const char* fieldName, DCore::DUInt& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(false);
	int value(out);
	if (ImGui::DragInt("##DragUInt", &value))
	{
		out = std::max(value, 0);
		toReturn = true;
	}
	ImGui::TreePop();
	return toReturn;
}

bool InspectorPanel::DrawFloatField(const char* fieldName, DCore::DFloat& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(false);
	if (ImGui::DragFloat("##DragFloat", &out))
	{
		toReturn = true;
	}
	ImGui::TreePop();
	return toReturn;
}

bool InspectorPanel::DrawLogicField(const char* fieldName, DCore::DLogic& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(false);
	if (ImGui::Checkbox("##Checkbox", &out))
	{
		toReturn = true;
	}
	ImGui::TreePop();
	return toReturn;
}
 
bool InspectorPanel::DrawVector2Field(const char* fieldName, const char* value1Label, const char* value2Label, DCore::DVec2& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(false);
	float drawWidth(ImGui::GetContentRegionAvail().x / 3.0f);
	ImGui::PushItemWidth(drawWidth);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", value1Label);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Drag Float 1", &out.x))
	{
		toReturn = true;
	}
	ImGui::SameLine();
	ImGui::Text("%s", value2Label);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Drag Float 2", &out.y))
	{
		toReturn = true;
	}
	ImGui::PopItemWidth();
	ImGui::TreePop();
	return toReturn;
}

bool InspectorPanel::DrawUIVector2Field(const char* fieldName, const char* value1Label, const char* value2Label, DCore::DVec2& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(false);
	int value[2];
	value[0] = out.x;
	value[1] = out.y;
	float drawWidth(ImGui::GetContentRegionAvail().x / 3.0f);
	ImGui::PushItemWidth(drawWidth);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", value1Label);
	ImGui::SameLine();
	if (ImGui::DragInt("##DragInt 1", &value[0]))
	{
		toReturn = true;
	}
	ImGui::SameLine();
	ImGui::Text("%s", value2Label);
	ImGui::SameLine();
	if (ImGui::DragInt("##DragInt 2", &value[1]))
	{
		toReturn = true;
	}
	ImGui::PopItemWidth();
	ImGui::TreePop();
	value[0] = std::max(value[0], 0);
	value[1] = std::max(value[1], 0);
	out.x = value[0];
	out.y = value[1];
	return toReturn;
}

bool InspectorPanel::DrawVector3Field(const char* fieldName, const char* value1Label, const char* value2Label, const char* value3Label, DCore::DVec3& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(false);
	float drawWidth(ImGui::GetContentRegionAvail().x / 5.0f);
	ImGui::PushItemWidth(drawWidth);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("%s", value1Label);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Drag Float 1", &out.x))
	{
		toReturn = true;
	}
	ImGui::SameLine();
	ImGui::Text("%s", value2Label);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Drag Float 2", &out.y))
	{
		toReturn = true;
	}
	ImGui::SameLine();
	ImGui::Text("%s", value3Label);
	ImGui::SameLine();
	if (ImGui::DragFloat("##Drag Floar 3", &out.z))
	{
		toReturn = true;
	}
	ImGui::PopItemWidth();
	ImGui::TreePop();
	return toReturn;
}

bool InspectorPanel::DrawStringField(const char* filedName, DCore::DString& out)
{
	return false;
}

bool InspectorPanel::DrawColorPickerField(const char* fieldName, DCore::DVec4& outColor)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(ImGui::ColorEdit4("##Color Picker", &outColor.x, ImGuiColorEditFlags_NoInputs));
	ImGui::TreePop();
	return toReturn;
}

bool InspectorPanel::DrawTaggedList(const char* fieldName, size_t currentTagIndex, const attributeNameType& attributeName, char* out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool toReturn(false);
	if (ImGui::BeginCombo("##Combo", attributeName.GetComponentAtIndex(currentTagIndex).c_str()))
	{
		for (size_t index(0); index < attributeName.GetNumberOfComponents(); index++)
		{
			const attributeNameType::stringType& tagName(attributeName.GetComponentAtIndex(index));
			if (ImGui::Selectable(tagName.c_str()))
			{
				toReturn = true;
				std::memcpy(out, tagName.c_str(), 256);
				break;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::TreePop();
	return toReturn;
}

bool InspectorPanel::DrawSpriteMaterialField(const char* fieldName, DCore::SpriteMaterialRef& spriteMaterial)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool returnValue(false);
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Clear"))
		{
			spriteMaterial = DCore::SpriteMaterialRef();
			returnValue = true;
		}
		ImGui::EndPopup();
	}
	if (spriteMaterial.IsValid())
	{
		ImGui::Text("%s", spriteMaterial.GetName().Data());
	}
	else
	{
		ImGui::Text("%s", "No sprite material");
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload(ImGui::AcceptDragDropPayload("SPRITE_MATERIAL_UUID"));
		if (payload != nullptr)
		{
			DCore::UUIDType& uuid(*(DCore::UUIDType*)payload->Data);
			spriteMaterial = MaterialManager::Get().LoadSpriteMaterial(uuid);
			returnValue = true;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::TreePop();
	return returnValue;
}

bool InspectorPanel::DrawImageButton(unsigned int textureId, const dVec2& size, const dVec4& tintColor)
{
	return ImGui::ImageButton("##Image Button", (ImTextureID)(intptr_t)textureId, ImVec2(size.x, size.y), {0, 1}, {1, 0}, {1, 1, 1, 1}, {tintColor.r, tintColor.g, tintColor.b, tintColor.a});
}

bool InspectorPanel::DrawColorPicker(ImVec4& outColor)
{
	return ImGui::ColorEdit4("##ColorPicker", &outColor.x, ImGuiColorEditFlags_NoInputs);
}

bool InspectorPanel::DrawPhysicsBodyType(const char* fieldName, bodyType& out)
{
	static const char* bodyTypeStrings[]{"Static", "Kinematic", "Dynamic"};
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool returnValue(false);
	if (ImGui::BeginCombo("##PhysicsBodyType", bodyTypeStrings[static_cast<size_t>(out)]))
	{
		for (uint8_t i(0); i < 3; i++)
		{
			if (ImGui::Selectable(bodyTypeStrings[i]))
			{
				returnValue = true;
				if (i == 0)
				{
					out = bodyType::Static;
				}
				else if (i == 1)
				{
					out = bodyType::Kinematic;
				}
				else
				{
					out = bodyType::Dynamic;
				}
			}
		}
		ImGui::EndCombo();
	}
	ImGui::TreePop();
	return returnValue;
}

bool InspectorPanel::DrawAnimationStateMachineField(const char* fieldName, coreAnimationStateMachineRefType animationStateMachine, coreAnimationStateMachineRefType& out)
{
	using uuidType = DCore::UUIDType;
	bool result(false);
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	ImGui::Text("%s", animationStateMachine.IsValid() ? animationStateMachine.GetName().c_str() : "No animation state machine");
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload(ImGui::AcceptDragDropPayload("ANIMATION_STATE_MACHINE_PAYLOAD"));
		if (payload != nullptr)
		{
			AnimationStateMachinePayload& animationStateMachinePayload(*static_cast<AnimationStateMachinePayload*>(payload->Data));
			coreAnimationStateMachineType newAnimationStateMachine(std::move(AnimationStateMachineManager::Get().LoadAnimationStateMachine(animationStateMachinePayload.UUID, animationStateMachinePayload.Name.Data()).GetCoreAnimationStateMachine()));
			out = DCore::AssetManager::Get().AddAnimationStateMachine(std::move(newAnimationStateMachine));
			result = true;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::TreePop();
	return result;
}

bool InspectorPanel::DrawPhysicsMaterialField(const char* fieldName, physicsMaterialRefType currentPhysicsMaterial, physicsMaterialRefType& out)
{
	bool result(false);
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Clear"))
		{
			result = true;
		}
		ImGui::EndPopup();
	}
	ImGui::Text("%s", currentPhysicsMaterial.IsValid() ? currentPhysicsMaterial.GetName().c_str() : "No physics material");
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload(ImGui::AcceptDragDropPayload("PHYSICS_MATERIAL_PAYLOAD"));
		if (payload != nullptr)
		{
			PhysicsMaterialPayload& physicsMateriqalPayload(*static_cast<PhysicsMaterialPayload*>(payload->Data));
			out = PhysicsMaterialManager::Get().LoadPhysicsMaterial(physicsMateriqalPayload.UUID);
			result = true;
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::TreePop();
	return result;
}

bool InspectorPanel::DrawPhysicsLayerField(const char* fieldName, physicsLayerType& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool returnValue(false);
	if (ImGui::BeginCombo("##PhysicsLayers", TypeNames::physicsLayerNames[static_cast<uint64_t>(out) == 0 ? 0 : (1 + glm::log2(static_cast<uint64_t>(out)))]))
	{
		for (size_t i(0); i < DCore::Physics::numberOfPhysicsLayers; i++)
		{
			if (ImGui::Selectable(TypeNames::physicsLayerNames[i]))
			{
				if (i == 0)
				{
					out = physicsLayerType::Unspecified;
				}
				else
				{
					out = static_cast<physicsLayerType>(1<<(i-1));
				}
				returnValue = true;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::TreePop();
	return returnValue;
}

bool InspectorPanel::DrawPhysicsLayersField(const char* fieldName, physicsLayerType& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool returnValue(false);
	if (ImGui::Button("Add"))
	{
		ImGui::OpenPopup("AddPhysicsLayerPopup");
	}
	if (ImGui::BeginPopup("AddPhysicsLayerPopup"))
	{
		for (size_t i(0); i < DCore::Physics::numberOfPhysicsLayers - 1; i++)
		{
			if (static_cast<uint64_t>(out) & 1<<i)
			{
				continue;
			}
			if (ImGui::Selectable(TypeNames::physicsLayerNames[i + 1]))
			{
				out = static_cast<physicsLayerType>(static_cast<uint64_t>(out) | 1<<i);
				returnValue = true;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::Indent();
	uint64_t removeMask(static_cast<uint64_t>(out));
	for (size_t i(0); i < DCore::Physics::numberOfPhysicsLayers - 1; i++)
	{
		if (static_cast<uint64_t>(out) & 1<<i)
		{
			ImGui::Selectable(TypeNames::physicsLayerNames[i + 1]);
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::Selectable("Remove"))
				{
					removeMask ^= 1<<i;
					returnValue = true;
				}
				ImGui::EndPopup();
			}
		}
	}
	out = static_cast<physicsLayerType>(static_cast<uint64_t>(out) & removeMask);
	ImGui::Unindent();
	ImGui::TreePop();
	return returnValue;
}

bool InspectorPanel::DrawSoundEventInstanceField(const char* fieldName, soundEventInstanceType& out)
{
	if (!ImGui::TreeNodeEx(fieldName, ImGuiTreeNodeFlags_SpanAvailWidth))
	{
		return false;
	}
	bool returnValue(false);
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::Selectable("Clear"))
		{
			returnValue = true;
			out.Internal_SetPath("");
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginCombo("##Paths", out.GetPath().Data()))
	{
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
				FMOD::Studio::EventDescription** eventDescriptions(static_cast<FMOD::Studio::EventDescription**>(alloca(count * sizeof(char*))));
				result = bank->getEventList(eventDescriptions, count, nullptr);
				DASSERT_E(result == FMOD_OK);
				for (size_t i(0); i < count; i++)
				{
					FMOD::Studio::EventDescription* eventDescription(eventDescriptions[i]);
					int pathSize(0);
					result = eventDescription->getPath(nullptr, 0, &pathSize);
					DASSERT_E(result == FMOD_OK);
					char* path(static_cast<char*>(alloca(pathSize * sizeof(char))));
					result = eventDescription->getPath(path, pathSize, nullptr);
					if (ImGui::Selectable(path))
					{
						returnValue = true;
						out.Internal_SetPath(path);
						out.Internal_SetBank(bank);
						return true;
					}
				}
				return false;
			}
		);	
		ImGui::EndCombo();
	}
	ImGui::TreePop();
	return returnValue;
}

}
