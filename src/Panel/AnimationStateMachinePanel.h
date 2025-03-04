#pragma once

#include "Panel.h"
#include "EditorAnimationStateMachine.h"

#include "DommusCore.h"

#include "imgui.h"

#include <vector>
#include <string>
#include <functional>



namespace DEditor
{

class AnimationStateMachinePanel : public Panel
{
public:
	using editorAnimationStateMachineType = EditorAnimationStateMachine;
	using dVec2 = DCore::DVec2;
	using stateRefType = editorAnimationStateMachineType::stateRefType;
	using stateConstRefType = editorAnimationStateMachineType::stateConstRefType;
	using createStateResult = EditorAnimationStateMachine::createStateResult;
	using renameStateResult = EditorAnimationStateMachine::renameStateResult;
	using parameterType = EditorAnimationStateMachine::parameterType;
	using transitionConstRefType = DCore::transitionConstRefType;
	using integerParameterConstRefType = DCore::integerParameterConstRefType;
	using floatParameterConstRefType = DCore::floatParameterConstRefType;
	using logicParameterConstRefType = DCore::logicParameterConstRefType;
	using triggerParameterConstRefType = DCore::triggerParameterConstRefType;
	using animationStateMachinePanelContainerType = DCore::ReciclingVector<AnimationStateMachinePanel>;
	using entityRefType = DCore::EntityRef;
	using uuidType = DCore::UUIDType;
	using stringType = std::string;
	using statesBeingMovedContainerType = std::vector<bool>;
	using selectedStatesContainerType = std::vector<bool>;
public:
	using ParameterInfo = struct ParameterInfo
	{
		parameterType Type;	
		size_t Index;

		ParameterInfo& operator=(const ParameterInfo& other)
		{
			Type = other.Type;
			Index = other.Index;
			return *this;
		}
	};
public:
	using parameterInfoContainerType = std::vector<ParameterInfo>;
public:
	static constexpr size_t stateNameSize{128};
	static constexpr size_t parameterNameSize{128};
public:
	AnimationStateMachinePanel(const stringType& panelName, editorAnimationStateMachineType&&, parameterInfoContainerType&&);
	AnimationStateMachinePanel(AnimationStateMachinePanel&&) noexcept;
	~AnimationStateMachinePanel() = default;
public:
	static void OpenAnimationStateMachinePanel(const stringType& animationStateMachineName, editorAnimationStateMachineType&&, parameterInfoContainerType&&);
	static void CloseAnimationStateMachinePanelWithUUID(const DCore::UUIDType&);
	static bool IsAnimationStateMachinePanelOpened(const uuidType& animationStateMachineUUID);
	static void RenderPanels();
public:
	// Returns if the panel have to be closed.
	bool Render();
public:
	const stringType& GetPanelName() const
	{
		return m_panelName;
	}

	const uuidType& GetAnimationUUID() const
	{
		return m_editorAnimationStateMachine.GetUUID();
	}

	editorAnimationStateMachineType& GetEditorAnimationStateMachine()
	{
		return m_editorAnimationStateMachine;
	}
	
	void SetContainerIndex(size_t index)
	{
		m_containerIndex = index;
	}
private:
	static animationStateMachinePanelContainerType s_animationStateMachinePanels;
private:
	size_t m_containerIndex;
	stringType m_panelName;
	bool m_isOpened;
	editorAnimationStateMachineType m_editorAnimationStateMachine;
	dVec2 m_scrolling;
	float m_pixelsPerUnit;
	float m_stepSize; // In units
	int m_stepSizeOrder;
	bool m_isMouseDraggingWithRightButton;
	selectedStatesContainerType m_selectedStatesMask;
	// State creation 
	char m_stateName[stateNameSize];
	bool m_isStateNameEmpty;
	bool m_isStateNameNotUnique;
	
	// Parameter
	char m_parameterName[parameterNameSize];
	bool m_isParameterNameEmpty;
	bool m_isParameterNameNotUnique;
	parameterType m_parameterTypeToAdd;
	bool m_isParameterNameBeingEdited;
	parameterInfoContainerType m_parameterInfos; // Used to draw parameters in a controllable sequence.
	size_t m_selectedParameterInfoIndex;
	//
	// State movement
	bool m_isDraggingState;
	//
	// State action menu
	stateConstRefType m_stateWithActionMenuOpened;
	//
	// Transition
	bool m_isCreatingTransition;
	size_t m_fromStateIndex;
	dVec2 m_fromStateCenterPos;
	size_t m_indexOfStateWithTransitionWithActionMenuOpened;
	size_t m_indexOfTransitionWithActionMenuOpened;
	transitionConstRefType m_selectedTransition;
	//
	// Simulation
	entityRefType m_attachedEntity;
	bool m_isPlaying;
private:
	enum class StateAction
	{
		Create,
		Rename
	};
	
	enum class DrawStateActionResult
	{
		Working,
		Done,
		Canceled
	};
private:
	void DrawParameters();
	void DrawSimulation();
	void DrawCanvas();
	void DrawStates(const dVec2& canvasCenterPos, const dVec2& canvasCenterValue);
	DrawStateActionResult DrawStateAction(StateAction, stateConstRefType* maybeState = nullptr, const dVec2* maybeStatePosition = nullptr);
	void DeselectAllStates();
	void DrawParameterNameEntry(std::function<void()> action);
	void CreateParameter();
	void UpdateSimulation();
private:
	template <class ParameterRefType>
	void DrawParameterDragDrop(
		ParameterRefType parameterRef,
		const ParameterInfo& parameterInfo,
		size_t parameterInfoIndex)
	{
		static const char* payloadLabel{ "PARAMETER_INFO" };
		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(payloadLabel, &parameterInfoIndex, sizeof(size_t));
			ImGui::Text("%s", parameterRef->GetName().c_str());
			ImGui::EndDragDropSource();
			m_selectedParameterInfoIndex = 0;
		}
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload(ImGui::AcceptDragDropPayload(payloadLabel));
			if (payload != nullptr)
			{
				const size_t fromParameterIndex(*static_cast<size_t*>(payload->Data));
				DASSERT_E(fromParameterIndex < m_parameterInfos.size());
				ParameterInfo temp(parameterInfo);
				m_parameterInfos[parameterInfoIndex] = m_parameterInfos[fromParameterIndex];
				m_parameterInfos[fromParameterIndex] = temp;
			}
			ImGui::EndDragDropTarget();
		}
	}

	template <class ParameterRefType>
	void DrawParameterActionMenu(
		ParameterRefType parameterRef, 
		size_t parameterInfoIndex, 
		std::vector<size_t>& parameterInfosToRemove)
	{
		static const ImGuiSelectableFlags popupSelectableFlags{ImGuiSelectableFlags_DontClosePopups};
		if (!m_isParameterNameBeingEdited && ImGui::BeginPopupContextItem())
		{
			m_isParameterNameBeingEdited = true;
			if (ImGui::Selectable("Rename", false, popupSelectableFlags))
			{
				ImGui::OpenPopup("ParameterActionMenuPopup");
				strcpy(m_parameterName, parameterRef->GetName().c_str());
			}
			if (ImGui::Selectable("Delete"))
			{
				parameterInfosToRemove.push_back(parameterInfoIndex);
				m_editorAnimationStateMachine.DeleteParameter(parameterRef);
			}
			DrawParameterNameEntry([&]() -> void { RenameParameter(parameterRef); });
			ImGui::EndPopup();
		}
	}

	template <class ParameterRefType>
	void RenameParameter(ParameterRefType parameter)
	{
		using renameParameterResult = EditorAnimationStateMachine::renameParameterResult;
		renameParameterResult result(m_editorAnimationStateMachine.RenameParameter<ParameterRefType::valueType::parameterType>(parameter.GetIndex(), m_parameterName));
		switch (result)
		{
		case renameParameterResult::ParameterNameEmpty:
			m_isParameterNameEmpty = true;
			return;
		case renameParameterResult::ParameterWithSameName:
			m_isParameterNameNotUnique = true;
			return;
		default:
			return;
		}
	}
};

}
