#pragma once

#include "EditorAnimationStateMachine.h"
#include "AnimationStateMachinePanel.h"
#include "MaterialManager.h"

#include "DommusCore.h"

#include "imgui.h"

#include <sstream>



namespace DEditor
{

class EditorAnimationStateMachine;

class InspectorPanel
{
	friend class Panels;
	friend class UtilityPanel;
public:
	using animationStateMachineType = EditorAnimationStateMachine;
	using asmPanelRefType = AnimationStateMachinePanel::animationStateMachinePanelContainerType::Ref;
	using entityRefType = DCore::EntityRef;
	using stringType = DCore::DString;
	using spriteMaterialRefType = DCore::SpriteMaterialRef;
	using texture2DType = DCore::Texture2D;
	using dInt = DCore::DInt;
	using dUInt = DCore::DUInt;
	using dFloat = DCore::DFloat;
	using dLogic = DCore::DLogic;
	using dVec2 = DCore::DVec2;
	using dVec3 =  DCore::DVec3;
	using dVec4 = DCore::DVec4;
	using stateConstRefType = DCore::AnimationStateMachine::stateConstRefType;
	using transitionConstRefType = DCore::transitionConstRefType;
	using coreAnimationStateMachineType = DCore::AnimationStateMachine;
	using conditionConstRefType = DCore::conditionConstRefType;
	using coreAnimationStateMachineRefType = DCore::AnimationStateMachineRef;
	using componentRefType = DCore::ComponentRef<DCore::Component>;
	using serializedAttributeType = DCore::SerializedAttribute;
	using attributeNameType = DCore::AttributeName;
	using physicsMaterialRefType = DCore::PhysicsMaterialRef;
	using bodyType = DCore::DBodyType;
	using physicsLayerType = DCore::Physics::PhysicsLayer;
	using soundEventInstanceType = DCore::SoundEventInstance;
	using parameterType = DCore::ParameterType;
	using integerParameterConstRefType = DCore::integerParameterConstRefType;
	using floatParameterConstRefType = DCore::floatParameterConstRefType;
	using logicParameterConstRefType = DCore::logicParameterConstRefType;
	using triggerParameterConstRefType = DCore::triggerParameterConstRefType;
public:
	static constexpr float comboWidthFactor{0.2f};
public:
	~InspectorPanel() = default;
private:
	enum class ContextType
	{
		Default,
		Entity,
		SpriteMaterial,
		Transition,
		State
	};
private:
	using Context = struct Context
	{
		using UniSpriteMaterial = struct UniSpriteMaterial
		{
			stringType Name;
			spriteMaterialRefType Ref;

			UniSpriteMaterial(const DCore::DString& name, spriteMaterialRefType ref)
				:
				Name(name.Data()),
				Ref(ref)
			{}
		};

		using AnimationStateMachineTransitionContent = struct AnimationStateMachineTransitionContent
		{
			size_t TransitionIndex;
			size_t FromStateIndex;
			size_t IdOfConditionWithActionMenuOpened;
			asmPanelRefType AsmPanel;

			AnimationStateMachineTransitionContent(
				size_t transitionIndex,
				size_t fromStateIndex,
				asmPanelRefType asmPanel)
				:
				TransitionIndex(transitionIndex),
				FromStateIndex(fromStateIndex),
				IdOfConditionWithActionMenuOpened(0),
				AsmPanel(asmPanel)
			{}
		};

		using AnimationStateMachineStateContent = struct AnimationStateMachineStateContent
		{
			stateConstRefType State;
			animationStateMachineType& AnimationStateMachine;

			AnimationStateMachineStateContent(
				stateConstRefType state,
				animationStateMachineType& animationStateMachine)
				:
				State(state),
				AnimationStateMachine(animationStateMachine)
			{}
		};

		using UniContent = union Content
		{
		public:
			using animationStateMachineTransitionContent = InspectorPanel::Context::AnimationStateMachineTransitionContent;
			using animationStateMachineStateContent = InspectorPanel::Context::AnimationStateMachineStateContent;
		public:
			entityRefType Entity;
			UniSpriteMaterial SpriteMaterial;
			animationStateMachineTransitionContent AnimationStateMachineTransitionContent;
			animationStateMachineStateContent AnimationStateMachineStateContent;
		public:
			Content()
			{
				std::memset(this, 0, sizeof(Content));
			}

			Content(entityRefType entity)
				:
				Entity(entity)
			{}

			Content(const stringType& name, spriteMaterialRefType spriteMaterial)
				:
				SpriteMaterial(name, spriteMaterial)
			{}

			Content(
				size_t transitionIndex, 
				size_t fromStateIndex,
				asmPanelRefType asmPanel)
				:
				AnimationStateMachineTransitionContent(transitionIndex, fromStateIndex, asmPanel)
			{}

			Content(stateConstRefType state,
					animationStateMachineType& animationStateMachine)
				:
				AnimationStateMachineStateContent(state, animationStateMachine)
			{}

			void SetEntity(entityRefType entityRef, ContextType currentType)
			{
				new (this) UniContent(entityRef);
			}

			void SetSpriteMaterial(const stringType& spriteMaterialName, spriteMaterialRefType spriteMaterialRef, ContextType currentType)
			{
				new (this) UniContent(spriteMaterialName, spriteMaterialRef);
			}
			
			void SetTransition(
					size_t transitionIndex,
					size_t fromStateIndex,
					asmPanelRefType asmPanel, 
					ContextType currentType)
			{
				new (this) UniContent(transitionIndex, fromStateIndex, asmPanel);
			}

			void SetState(stateConstRefType state,
					animationStateMachineType& animationStateMachine, ContextType currentType)
			{
				new (this) UniContent(state, animationStateMachine);
			}
		};

		ContextType Type;
		UniContent Content;
		
		Context()
			:
			Type(ContextType::Default)
		{}
		
		void SetEntity(entityRefType entity)
		{
			Content.SetEntity(entity, Type);
			Type = ContextType::Entity;
		}

		void SetSpriteMaterial(const stringType& spriteMaterialName, spriteMaterialRefType spriteMaterial)
		{
			Content.SetSpriteMaterial(spriteMaterialName, spriteMaterial, Type);
			Type = ContextType::SpriteMaterial;
		}

		void SetTranstion(
			size_t transitionIndex, 
			size_t fromStateIndex,
			asmPanelRefType& asmPanel)
		{
			Content.SetTransition(transitionIndex, fromStateIndex, asmPanel, Type);
			Type = ContextType::Transition;
		}
		
		void SetState(stateConstRefType state, 
				animationStateMachineType& animationStateMachine)
		{
			Content.SetState(state, animationStateMachine, Type);
			Type = ContextType::State;
		}
	};
public:
	static InspectorPanel& Get()
	{
		static InspectorPanel entityAttributesPanel;
		return entityAttributesPanel;
	}
public:
	void Clear()
	{
		m_renderFunction = nullptr;
	}
public:
	void DisplayEntity(entityRefType);
	void DisplayTransition(size_t transitionIndex, size_t fromStateIndex, asmPanelRefType);
	void DisplayState(stateConstRefType, animationStateMachineType&);
private:
	InspectorPanel();
private:
	bool m_isOpened;
	texture2DType m_whiteMap;
	Context m_context;
private:
	void Open();
	void Render();
private:
	void (InspectorPanel::*m_renderFunction)(bool);
	void RenderEntity(bool toSetup);
	void RenderTransition(bool toSetup);
	void RenderState(bool toSetup);
private:
	bool DrawIntegerField(const char* fieldName, dInt&);
	bool DrawUIntegerField(const char* fieldName, dUInt&);
	bool DrawFloatField(const char* fieldName, dFloat&);
	bool DrawLogicField(const char* fieldName, dLogic&);
	bool DrawVector2Field(const char* fieldName, const char* value1Label, const char* value2Label, dVec2&);
	bool DrawUIVector2Field(const char* fieldName, const char* value1Label, const char* value2Label, dVec2&);
	bool DrawVector3Field(const char* fieldName, const char* value1Label, const char* value2Label, const char* value3Label, dVec3&);
	bool DrawStringField(const char* fieldName, stringType&);
	bool DrawColorPickerField(const char* fieldName, dVec4& outColor);
	bool DrawTaggedList(const char* fieldName, size_t currentTagIndex, const attributeNameType&, char* out);
	bool DrawSpriteMaterialField(const char* fieldName, spriteMaterialRefType&);
	bool DrawImageButton(unsigned int textureId, const dVec2& size, const dVec4& tintColor);														// Its not unique by default.
	bool DrawColorPicker(ImVec4& outColor);																											// Its not unique by default.			
	bool DrawPhysicsBodyType(const char* fieldName, bodyType& out);
	bool DrawAnimationStateMachineField(const char* fieldName, coreAnimationStateMachineRefType, coreAnimationStateMachineRefType& out);
	bool DrawPhysicsMaterialField(const char* fieldName, physicsMaterialRefType currentPhysicsMaterial, physicsMaterialRefType& out);
	bool DrawPhysicsLayerField(const char* fieldName, physicsLayerType& out);
	bool DrawPhysicsLayersField(const char* fieldName, physicsLayerType& out);
	bool DrawSoundEventInstanceField(const char* fieldName, soundEventInstanceType& out);
private:
	template <class ParameterRefType, class CurrentParameterRefType>
	bool DrawParameter(
		ParameterRefType parameter, 
		size_t fromStateIndex, 
		size_t transitionIndex, 
		animationStateMachineType& animationStateMachine,
		conditionConstRefType localCondition,
		CurrentParameterRefType currentParameter)
	{
		parameterType type(parameter->GetType());
		if (type == currentParameter->GetType() && parameter.GetIndex() == currentParameter.GetIndex())
		{
			return false;
		}
		if (ImGui::Selectable(parameter->GetName().c_str()))
		{
			animationStateMachine.SetConditionParameter(fromStateIndex, transitionIndex, localCondition.GetIndex(), type, parameter.GetIndex());
			return true;
		}
		return false;
	}

	template <class ParameterRefType>
	bool DrawParameterSelection(
		conditionConstRefType localCondition, 
		ParameterRefType currentParameter,
		animationStateMachineType& animationStateMachine,
		stateConstRefType fromState,
		stateConstRefType toState,
		transitionConstRefType transition)
	{
		static_assert(std::is_same_v<integerParameterConstRefType, decltype(currentParameter)> ||
						std::is_same_v<floatParameterConstRefType, decltype(currentParameter)> ||
						std::is_same_v<logicParameterConstRefType, decltype(currentParameter)> ||
						std::is_same_v<triggerParameterConstRefType, decltype(currentParameter)>,
						"Invalid currentParameter arg.");
		const size_t conditionIndex(localCondition.GetIndex());
		const ImVec2 regionAvailable(ImGui::GetContentRegionAvail());
		std::stringstream conditionLabelStream;
		std::stringstream parameterLabelStream;
		conditionLabelStream << "-##" << conditionIndex;
		parameterLabelStream << "##P" << conditionIndex;
		ImGui::AlignTextToFramePadding();
		if (ImGui::Selectable(conditionLabelStream.str().c_str(), false, ImGuiSelectableFlags_AllowOverlap))
		{
			// TODO. Condition selection.
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("ConditionActionMenuPopup");
			m_context.Content.AnimationStateMachineTransitionContent.IdOfConditionWithActionMenuOpened = localCondition.GetId();
		}
		if (
			localCondition.GetId() == m_context.Content.AnimationStateMachineTransitionContent.IdOfConditionWithActionMenuOpened && 
			ImGui::BeginPopup("ConditionActionMenuPopup"))
		{
			bool conditionDeleted(false);
			if (ImGui::Selectable("Delete"))
			{
				animationStateMachine.DeleteCondition(transition->GetFromStateIndex(), transition.GetIndex(), conditionIndex);
				conditionDeleted = true;
			}
			ImGui::EndPopup();
			if (conditionDeleted)
			{
				return true;
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(regionAvailable.x * comboWidthFactor);
		bool parameterTypeChanged(false);
		if (ImGui::BeginCombo(parameterLabelStream.str().c_str(), currentParameter->GetName().c_str()))
		{
			animationStateMachine.IterateOnParameters<parameterType::Integer>
			(
				[&](integerParameterConstRefType parameter) -> bool
				{
					parameterTypeChanged = DrawParameter(
							parameter, 
							fromState.GetIndex(), 
							transition.GetIndex(), 
							animationStateMachine, 
							localCondition, 
							currentParameter);
					return parameterTypeChanged;
				}
			);
			if (parameterTypeChanged)
			{
				ImGui::EndCombo();
				return true;
			}
			animationStateMachine.IterateOnParameters<parameterType::Float>
			(
				[&](floatParameterConstRefType parameter) -> bool
				{
					parameterTypeChanged = DrawParameter(
							parameter, 
							fromState.GetIndex(), 
							transition.GetIndex(), 
							animationStateMachine, 
							localCondition, 
							currentParameter);
					return parameterTypeChanged;
				}
			);
			if (parameterTypeChanged)
			{
				ImGui::EndCombo();
				return true;
			}
			animationStateMachine.IterateOnParameters<parameterType::Logic>
			(
				[&](logicParameterConstRefType parameter) -> bool
				{
					parameterTypeChanged = DrawParameter(
							parameter, 
							fromState.GetIndex(), 
							transition.GetIndex(), 
							animationStateMachine, 
							localCondition, 
							currentParameter);
					return parameterTypeChanged;
				}
			);
			if (parameterTypeChanged)
			{
				ImGui::EndCombo();
				return true;
			}
			animationStateMachine.IterateOnParameters<parameterType::Trigger>
			(
				[&](triggerParameterConstRefType parameter) -> bool
				{
					parameterTypeChanged = DrawParameter(
							parameter, 
							fromState.GetIndex(), 
							transition.GetIndex(), 
							animationStateMachine, 
							localCondition, 
							currentParameter);
					return parameterTypeChanged;
				}
			);
			ImGui::EndCombo();
		}
		return parameterTypeChanged;
	}
};

}
