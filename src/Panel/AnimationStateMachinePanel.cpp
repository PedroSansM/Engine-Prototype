#include "AnimationStateMachinePanel.h"
#include "Log.h"
#include "InspectorPanel.h"
#include "AnimationStateMachineManager.h"
#include "Draw.h"

#include "DCoreMath.h"
#include "imgui.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iterator>
#include <stack>
#include <sstream>



namespace DEditor
{

AnimationStateMachinePanel::animationStateMachinePanelContainerType AnimationStateMachinePanel::s_animationStateMachinePanels;

AnimationStateMachinePanel::AnimationStateMachinePanel(const stringType& panelName, editorAnimationStateMachineType&& editorAnimationStateMachine, parameterInfoContainerType&& parameterInfos)
	:
	m_panelName(panelName),
	m_isOpened(true),
	m_editorAnimationStateMachine(std::move(editorAnimationStateMachine)),
	m_scrolling(0.0f, 0.0f),
	m_pixelsPerUnit(128.0f),
	m_stepSize(1.0f),
	m_stepSizeOrder(0),
	m_isMouseDraggingWithRightButton(false),
	m_isStateNameEmpty(false),
	m_isStateNameNotUnique(false),
	m_isParameterNameEmpty(false),
	m_isParameterNameNotUnique(false),
	m_parameterTypeToAdd(parameterType::Float),
	m_isParameterNameBeingEdited(false),
	m_parameterInfos(std::move(parameterInfos)),
	m_selectedParameterInfoIndex(0),
	m_isDraggingState(false),
	m_fromStateIndex(0),
	m_isCreatingTransition(false),
	m_indexOfStateWithTransitionWithActionMenuOpened(0),
	m_indexOfTransitionWithActionMenuOpened(0),
	m_isPlaying(false)
{
	std::memset(m_stateName, 0, stateNameSize);
	std::memset(m_parameterName, 0, parameterNameSize);
	m_selectedStatesMask.clear();
	m_selectedStatesMask.resize(m_editorAnimationStateMachine.GetNumberOfStates(), false);
}

AnimationStateMachinePanel::AnimationStateMachinePanel(AnimationStateMachinePanel&& other) noexcept
	:
	m_panelName(std::move(other.m_panelName)),
	m_isOpened(true),
	m_editorAnimationStateMachine(std::move(other.m_editorAnimationStateMachine)),
	m_scrolling(other.m_scrolling),
	m_pixelsPerUnit(other.m_pixelsPerUnit),
	m_stepSize(other.m_stepSize),
	m_stepSizeOrder(other.m_stepSizeOrder),
	m_isMouseDraggingWithRightButton(other.m_isMouseDraggingWithRightButton),
	m_isStateNameEmpty(other.m_isStateNameEmpty),
	m_isStateNameNotUnique(other.m_isStateNameNotUnique),
	m_isParameterNameEmpty(other.m_isParameterNameEmpty),
	m_isParameterNameNotUnique(other.m_isParameterNameNotUnique),
	m_parameterTypeToAdd(other.m_parameterTypeToAdd),
	m_selectedStatesMask(std::move(other.m_selectedStatesMask)),
	m_isParameterNameBeingEdited(other.m_isParameterNameBeingEdited),
	m_parameterInfos(std::move(other.m_parameterInfos)),
	m_selectedParameterInfoIndex(other.m_selectedParameterInfoIndex),
	m_isDraggingState(other.m_isDraggingState),
	m_isCreatingTransition(other.m_isCreatingTransition),
	m_fromStateIndex(other.m_fromStateIndex),
	m_fromStateCenterPos(other.m_fromStateCenterPos),
	m_indexOfStateWithTransitionWithActionMenuOpened(other.m_indexOfStateWithTransitionWithActionMenuOpened),
	m_indexOfTransitionWithActionMenuOpened(other.m_indexOfTransitionWithActionMenuOpened),
	m_isPlaying(false)
{
	std::memset(m_stateName, 0, stateNameSize);
	std::memset(m_parameterName, 0, parameterNameSize);
}

void AnimationStateMachinePanel::OpenAnimationStateMachinePanel(const stringType& animationStateMachineName, editorAnimationStateMachineType&& editorAnimationStateMachine, parameterInfoContainerType&& parameterInfos)
{
	animationStateMachinePanelContainerType::Ref asmPanel(s_animationStateMachinePanels.PushBack(animationStateMachineName, std::move(editorAnimationStateMachine), std::move(parameterInfos)));
	asmPanel->SetContainerIndex(asmPanel.GetIndex());
}

void AnimationStateMachinePanel::CloseAnimationStateMachinePanelWithUUID(const DCore::UUIDType& uuid)
{
	s_animationStateMachinePanels.Iterate
	(
		[&](animationStateMachinePanelContainerType::ConstRef asmPanel) -> bool
		{
			if (asmPanel->GetAnimationUUID() == uuid)
			{
				s_animationStateMachinePanels.Remove(asmPanel);
				return true;
			}
			return false;
		}
	);
}

bool AnimationStateMachinePanel::IsAnimationStateMachinePanelOpened(const uuidType& animationStateMachineUUID)
{
	bool isOpened(false);
	s_animationStateMachinePanels.Iterate
	(
		[&](animationStateMachinePanelContainerType::ConstRef animationStatemachinePanel) -> bool
		{
			if (animationStatemachinePanel->GetAnimationUUID() == animationStateMachineUUID)
			{
				isOpened = true;
				return true;
			}
			return false;
		}
	);
	return isOpened;
}

void AnimationStateMachinePanel::RenderPanels()
{
	s_animationStateMachinePanels.Iterate
	(
		[&](animationStateMachinePanelContainerType::Ref animationStateMachinePanel) -> bool
		{
			if (animationStateMachinePanel->Render())
			{
				s_animationStateMachinePanels.Remove(animationStateMachinePanel);
			}
			return false;
		}
	);
}

bool AnimationStateMachinePanel::Render()
{
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	if (!ImGui::Begin(m_panelName.c_str(), &m_isOpened, windowFlags))
	{
		ImGui::End();
		return false;
	}
	if (!m_isOpened)
	{
		InspectorPanel::Get().Clear();
		ImGui::End();
		return true;
	}
	const ImVec2 regionAvail(ImGui::GetContentRegionAvail());
	if (ImGui::BeginChild("Parameters", {regionAvail.x * 0.4f, regionAvail.y}, ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border))
	{
		if (ImGui::Button("Save"))
		{
			AnimationStateMachineManager::Get().SaveChanges(m_editorAnimationStateMachine, m_parameterInfos);
		}
		DrawParameters();
		DrawSimulation();
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("Canvas", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		DrawCanvas();
	}
	ImGui::EndChild();
	ImGui::End();
	UpdateSimulation();
	return false;
}

void AnimationStateMachinePanel::DrawParameters()
{
	using integerParameterConstRefType = DCore::integerParameterContainerType::ConstRef;
	using floatParameterConstRefType = DCore::floatParameterContainerType::ConstRef;	
	using logicParameterConstRefType = DCore::logicParameterContainerType::ConstRef;
	using triggerParameterConstRefType = DCore::triggerParameterContainerType::ConstRef;
	using logicParameterType = DCore::LogicParameter;
	using triggerParameterType = DCore::TriggerParameter;
	if (ImGui::Button("Add"))
	{
		ImGui::OpenPopup("AddParameterPopup");
	}
	if (ImGui::BeginPopup("AddParameterPopup"))
	{
		static constexpr uint8_t numberOfParameters(4);
		static constexpr parameterType parameterTypes[numberOfParameters]{parameterType::Integer, parameterType::Float, parameterType::Logic, parameterType::Trigger};
		static const char* parameterTypesString[numberOfParameters]{"Integer", "Float", "Logic", "Trigger"};
		for (uint8_t index(0); index < numberOfParameters; index++)
		{
			if (ImGui::Selectable(parameterTypesString[index], false, ImGuiSelectableFlags_DontClosePopups))
			{
				ImGui::OpenPopup("ParameterActionMenuPopup");
				std::memset(m_parameterName, 0, parameterNameSize);
				m_parameterTypeToAdd = parameterTypes[index];
			}
		}
		DrawParameterNameEntry([&]() -> void { return CreateParameter(); });
		ImGui::EndPopup();
	}
	if (ImGui::CollapsingHeader("Parameters"))
	{
		static const ImGuiSelectableFlags selectableFlags{ImGuiSelectableFlags_AllowOverlap};
		int integerValue(0);
		float floatValue(0.0f);
		logicParameterType logicValue;
		triggerParameterType triggerValue;
		char label[parameterNameSize];
		ImGui::Indent();
		if (ImGui::ArrowButton("##ParameterInfoUp", ImGuiDir_Up) && m_selectedParameterInfoIndex > 1 && m_selectedParameterInfoIndex - 1 < m_parameterInfos.size())
		{
			const size_t fromIndex(m_selectedParameterInfoIndex-- - 1);
			const size_t targetIndex(fromIndex - 1);
			ParameterInfo temp(m_parameterInfos[fromIndex]);
			m_parameterInfos[fromIndex] = m_parameterInfos[targetIndex];
			m_parameterInfos[targetIndex] = temp; 		
		}
		ImGui::SameLine();
		if (ImGui::ArrowButton("##ParameterInfoDown", ImGuiDir_Down) && m_selectedParameterInfoIndex > 0 && m_selectedParameterInfoIndex < m_parameterInfos.size())
		{
			const size_t fromIndex(m_selectedParameterInfoIndex++ - 1);
			const size_t targetIndex(fromIndex + 1);
			ParameterInfo temp(m_parameterInfos[fromIndex]);
			m_parameterInfos[fromIndex] = m_parameterInfos[targetIndex];
			m_parameterInfos[targetIndex] = temp; 				
		}
		ImGui::Unindent();
		std::vector<size_t> parameterInfosToRemove;
		for (size_t parameterInfoIndex(0); parameterInfoIndex < m_parameterInfos.size(); parameterInfoIndex++)
		{
			const ParameterInfo& parameterInfo(m_parameterInfos[parameterInfoIndex]);
			switch (parameterInfo.Type)
			{
			case parameterType::Integer:
			{
				integerParameterConstRefType parameter(m_editorAnimationStateMachine.TryGetParameterAtIndex<parameterType::Integer>(parameterInfo.Index));
				integerValue = parameter->GetValue();
				sprintf(label, "##%s", parameter->GetName().c_str());
				ImGui::Indent();
				ImGui::AlignTextToFramePadding();
				if (ImGui::Selectable(parameter->GetName().c_str(), (m_selectedParameterInfoIndex != 0 && m_selectedParameterInfoIndex - 1 == parameterInfoIndex), selectableFlags))
				{
					m_selectedParameterInfoIndex = parameterInfoIndex + 1;
				}
				DrawParameterDragDrop(parameter, parameterInfo, parameterInfoIndex);
				DrawParameterActionMenu(parameter, parameterInfoIndex, parameterInfosToRemove);
				ImGui::SameLine();
				if (ImGui::DragInt(label, &integerValue))
				{
					m_editorAnimationStateMachine.SetParameterValue<parameterType::Integer>(parameter.GetIndex(), integerValue);
				}
				ImGui::Unindent();
				break;
			}
			case parameterType::Float:
			{
				floatParameterConstRefType parameter(m_editorAnimationStateMachine.TryGetParameterAtIndex<parameterType::Float>(parameterInfo.Index));
				floatValue = parameter->GetValue();
				sprintf(label, "##%s", parameter->GetName().c_str());
				ImGui::Indent();
				ImGui::AlignTextToFramePadding();
				if (ImGui::Selectable(parameter->GetName().c_str(), (m_selectedParameterInfoIndex != 0 && m_selectedParameterInfoIndex - 1 == parameterInfoIndex), selectableFlags))
				{
					m_selectedParameterInfoIndex = parameterInfoIndex + 1;
				}
				DrawParameterDragDrop(parameter, parameterInfo, parameterInfoIndex);
				DrawParameterActionMenu(parameter, parameterInfoIndex, parameterInfosToRemove);
				ImGui::SameLine();
				if (ImGui::DragFloat(label, &floatValue, 0.1f))
				{
					m_editorAnimationStateMachine.SetParameterValue<parameterType::Float>(parameter.GetIndex(), floatValue);
				}
				ImGui::Unindent();
				break;
			}
			case parameterType::Logic:
			{
				logicParameterConstRefType parameter(m_editorAnimationStateMachine.TryGetParameterAtIndex<parameterType::Logic>(parameterInfo.Index));
				logicValue = parameter->GetValue();
				sprintf(label, "##%s", parameter->GetName().c_str());
				ImGui::Indent();
				ImGui::AlignTextToFramePadding();
				if (ImGui::Selectable(parameter->GetName().c_str(), (m_selectedParameterInfoIndex != 0 && m_selectedParameterInfoIndex - 1 == parameterInfoIndex), selectableFlags))
				{
					m_selectedParameterInfoIndex = parameterInfoIndex + 1;
				}
				DrawParameterDragDrop(parameter, parameterInfo, parameterInfoIndex);
				DrawParameterActionMenu(parameter, parameterInfoIndex, parameterInfosToRemove);
				ImGui::SameLine();
				if (ImGui::Checkbox(label, &logicValue.Value))
				{
					m_editorAnimationStateMachine.SetParameterValue<parameterType::Logic>(parameter.GetIndex(), logicParameterType{logicValue.Value});
				}
				ImGui::Unindent();
				break;
			}
			case parameterType::Trigger:
			{
				triggerParameterConstRefType parameter(m_editorAnimationStateMachine.TryGetParameterAtIndex<parameterType::Trigger>(parameterInfo.Index));
				triggerValue = parameter->GetValueWithoutReset();
				sprintf(label, "##%s", parameter->GetName().c_str());
				ImGui::Indent();
				ImGui::AlignTextToFramePadding();
				if (ImGui::Selectable(parameter->GetName().c_str(), (m_selectedParameterInfoIndex != 0 && m_selectedParameterInfoIndex - 1 == parameterInfoIndex), selectableFlags))
				{
					m_selectedParameterInfoIndex = parameterInfoIndex + 1;
				}
				DrawParameterDragDrop(parameter, parameterInfo, parameterInfoIndex);
				DrawParameterActionMenu(parameter, parameterInfoIndex, parameterInfosToRemove);
				ImGui::SameLine();
				ImGui::RadioButton(label, triggerValue.Value);
				ImGui::Unindent();
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				{
					triggerValue.Value = !triggerValue.Value;
					m_editorAnimationStateMachine.SetParameterValue<parameterType::Trigger>(parameter.GetIndex(), triggerParameterType{triggerValue.Value});
				}
				break;
			}
			}
		}
		m_isParameterNameBeingEdited = false;
		for (size_t parameterInfoIndex : parameterInfosToRemove)
		{
			m_parameterInfos.erase(m_parameterInfos.begin() + parameterInfoIndex);
		}
	}
}

void AnimationStateMachinePanel::DrawSimulation()
{
	using sceneRefType = DCore::SceneRef;
	using entityType = DCore::Entity;
	if (!ImGui::CollapsingHeader("Simulation"))
	{
		return;
	}
	ImGui::Indent();
	if (ImGui::Button("Attach"))
	{
		ImGui::OpenPopup("SelectEntityPopup");
	}
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	if (m_attachedEntity.IsValid())
	{
		ImGui::SameLine();
		if (ImGui::Button("Detach"))
		{
			m_attachedEntity.Invalidate();
			m_isPlaying = false;
		}
	}
	if (ImGui::BeginPopup("SelectEntityPopup"))
	{
		DCore::AssetManager::Get().IterateOnLoadedScenes
		(
			[&](sceneRefType scene) -> bool
			{
				scene.IterateOnEntities
				(
					[&](entityType entity) -> bool
					{
						entityRefType entityRef(entity, scene);
						std::stringstream labelStream;
						std::stack<entityRefType> parentStack;
						entityRef.IterateOnParents
						(
							[&](entityRefType parent) -> bool
							{
								parentStack.push(parent);
								return false;
							}
						);
						while (!parentStack.empty())
						{
							DCore::DString parentName;
							parentStack.top().GetName(parentName);
							labelStream << parentName.Data() << "->";
							parentStack.pop();
						}
						DCore::DString entityName;
						entityRef.GetName(entityName);
						labelStream << entityName.Data();
						if (ImGui::Selectable(labelStream.str().c_str()))
						{
							m_attachedEntity = entityRef;
							m_editorAnimationStateMachine.AttachTo(m_attachedEntity);
							return true;
						}
						return false;
					}
				);
				return false;
			}
		);
		ImGui::EndPopup();
	}
	if (m_attachedEntity.IsValid())
	{
		DCore::DString name;
		m_attachedEntity.GetName(name);
		ImGui::Text("Attachment: %s", name.Data());
		const ImVec2 cursorPos(ImGui::GetCursorScreenPos());
		if (!m_isPlaying)
		{
			static constexpr ImVec2 playSymbolSizes{80.0f, 50.0f};
			Draw::DrawPlaySymbol({playSymbolSizes.x, playSymbolSizes.y}, {cursorPos.x, cursorPos.y});
			if (ImGui::InvisibleButton("Play", playSymbolSizes))
			{
				m_editorAnimationStateMachine.Setup();
				m_isPlaying = true;
			}
		}
		else
		{
			static constexpr ImVec2 stopSymbolSizes{15.0f, 50.0f};
			static constexpr float stopSymbolDistance{10.0f};
			Draw::DrawStopSymbol({stopSymbolSizes.x, stopSymbolSizes.y}, stopSymbolDistance, {cursorPos.x, cursorPos.y});
			ImGui::SetCursorScreenPos(cursorPos);
			if (ImGui::InvisibleButton("Stop", {2 * stopSymbolSizes.x + stopSymbolDistance, stopSymbolSizes.y}))
			{
				m_isPlaying = false;
			}
		}
	}
	else
	{
		ImGui::Text("%s", "Attachment: NOT ATTACHED");
		m_isPlaying = false;
	}
	ImGui::Unindent();
}

void AnimationStateMachinePanel::DrawCanvas()
{
	ImGuiIO& io(ImGui::GetIO());
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	const ImVec2 canvasSize(ImGui::GetContentRegionAvail());
	const ImVec2 canvasMin(ImGui::GetCursorScreenPos());
	const ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);
	drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(50, 50, 50, 255));
	ImGui::SetNextItemAllowOverlap();
	ImGui::InvisibleButton("CanvasButton", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	const bool isActive(ImGui::IsItemActive());
	const bool isHovered(ImGui::IsItemHovered());
	const ImVec2 canvasCenterPos(canvasMin.x + canvasSize.x/2.0f, canvasMin.y + canvasSize.y/2.0f);
	if (isActive)
	{
		if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
		{
			m_scrolling.x += io.MouseDelta.x;
			m_scrolling.y += io.MouseDelta.y;
			m_isMouseDraggingWithRightButton = true;
		}
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			DeselectAllStates();
			m_selectedTransition.Invalidate();
			// Clear Inpector Panel.
			InspectorPanel::Get().Clear();
		}
	}
	if (isHovered)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_C))
		{
			m_isCreatingTransition = false;
		}
	}
	// Zoom
	if (isHovered && io.MouseWheel != 0.0f)
	{
		const float currentCenterValueX(-m_scrolling.x/m_pixelsPerUnit);
		const float currentCenterValueY(-m_scrolling.y/m_pixelsPerUnit);
		const float currentMouseValueX(currentCenterValueX + (io.MousePos.x - canvasCenterPos.x)/m_pixelsPerUnit);
		const float currentMouseValueY(currentCenterValueY + (io.MousePos.y - canvasCenterPos.y)/m_pixelsPerUnit);
		m_pixelsPerUnit *= io.MouseWheel == 1.0f ? 1.2f : 0.83333f;
		if (m_stepSize * m_pixelsPerUnit < 10.0f)
		{
			if (m_stepSizeOrder % 3 == 1)
			{
				m_stepSize += 3 * glm::pow(10, m_stepSizeOrder/3);
			}
			else if (m_stepSizeOrder % 3 == -2)
			{
				m_stepSize += 3 * glm::pow(10, m_stepSizeOrder/3 - 1);
			}
			else
			{
				m_stepSize *= 2.0f;
			}
			m_stepSizeOrder += 1;
		}
		else if (m_stepSize * m_pixelsPerUnit > 20.0f)
		{
			if (m_stepSizeOrder % 3 == 2)
			{
				m_stepSize -= 3 * glm::pow(10, m_stepSizeOrder/3);
			}
			else if (m_stepSizeOrder % 3 == -1)
			{
				m_stepSize -= 3 * glm::pow(10, m_stepSizeOrder/3 - 1);
			}
			else
			{
				m_stepSize /= 2.0f;
			}
			m_stepSizeOrder -= 1;
		}
		float nextCenterValueX(currentMouseValueX + (canvasCenterPos.x - io.MousePos.x)/m_pixelsPerUnit);
		float nextCenterValueY(currentMouseValueY + (canvasCenterPos.y - io.MousePos.y)/m_pixelsPerUnit);
		m_scrolling.x = -nextCenterValueX * m_pixelsPerUnit;
		m_scrolling.y = -nextCenterValueY * m_pixelsPerUnit;
	}
	const ImVec2 canvasCenterValue(-m_scrolling.x/m_pixelsPerUnit, -m_scrolling.y/m_pixelsPerUnit);
	// Actin menu
	if (isHovered && !m_isCreatingTransition && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
	{
		if (m_isMouseDraggingWithRightButton)
		{
			m_isMouseDraggingWithRightButton = false;
		}
		else
		{
			ImGui::OpenPopup("ActionMenuPopup");
		}
	}
	if (ImGui::BeginPopup("ActionMenuPopup"))
	{
		if (ImGui::Selectable("Create State", false, ImGuiSelectableFlags_DontClosePopups))
		{
			ImGui::OpenPopup("CreateStatePopup");
			std::memset(m_stateName, 0, stateNameSize);
		}
		if (ImGui::BeginPopup("CreateStatePopup"))
		{
			const dVec2 newStatePosition(canvasCenterValue.x, canvasCenterValue.y);
			const DrawStateActionResult result(DrawStateAction(StateAction::Create, nullptr, &newStatePosition));
			if (result == DrawStateActionResult::Done || result == DrawStateActionResult::Canceled)
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}
	//
	drawList->PushClipRect(canvasMin, canvasMax, true);
	float leftValue(canvasCenterValue.x + (canvasMin.x - canvasCenterPos.x)/m_pixelsPerUnit);	
	int nx(floor(leftValue/m_stepSize));
	float valueX(nx * m_stepSize);
	float x((valueX - leftValue) * m_pixelsPerUnit);
	float topValue(canvasCenterValue.y + (canvasMin.y - canvasCenterPos.y)/m_pixelsPerUnit);
	int ny(floor(topValue/m_stepSize));
	float valueY(ny * m_stepSize);
	float y((valueY - topValue) * m_pixelsPerUnit);
	for (float yi(y); yi < canvasSize.y; yi += m_stepSize * m_pixelsPerUnit)
	{
		drawList->AddLine({canvasMin.x, canvasMin.y + yi}, {canvasMax.x, canvasMin.y + yi}, IM_COL32(255, 255, 255, 20));
	}
	for (float xi(x); xi < canvasSize.x; xi += m_stepSize * m_pixelsPerUnit)
	{
		drawList->AddLine({canvasMin.x + xi, canvasMin.y}, {canvasMin.x + xi, canvasMax.y}, IM_COL32(255, 255, 255, 20));
	}
	DrawStates({canvasCenterPos.x, canvasCenterPos.y}, {canvasCenterValue.x, canvasCenterValue.y});
	drawList->PopClipRect();
}

void AnimationStateMachinePanel::DrawStates(const dVec2& canvasCenterPos, const dVec2& canvasCenterValue)
{
	using stateConstRefType = DCore::AnimationStateMachine::stateConstRefType;
	using transitionConstRefType = DCore::transitionContainerType::ConstRef;
	static constexpr dVec2 stateSize{3.0f, 1.0f};
	static constexpr dVec2 stateBorderSize{0.1, 0.1};
	const dVec2 stateSizePixels(stateSize * m_pixelsPerUnit);
	const dVec2 stateBorderSizePixels(stateBorderSize * m_pixelsPerUnit);
	ImGuiIO& io(ImGui::GetIO());
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	auto drawTransition = [&](const ImVec2& from, const ImVec2& to, bool isThisTransitionSelected) -> bool
	{
		using dVec3 = DCore::DVec3;
		constexpr ImU32 notSelectedColor(IM_COL32(255, 255, 255, 255));
		constexpr ImU32 selectedColor(IM_COL32(0, 0, 255, 255));
		const ImU32 color(isThisTransitionSelected ? selectedColor : notSelectedColor);
		drawList->AddLine(to, from, color);
		constexpr float arrowRatio{0.49f};
		constexpr dVec2 arrowSize{0.2f, 0.2f}; // Width, Heigth.
		const dVec2 arrowSizePixels{arrowSize.x * m_pixelsPerUnit, arrowSize.y * m_pixelsPerUnit};
		const dVec2 difference{to.x - from.x, to.y - from.y};
		const float lineLength(std::sqrt(std::pow(difference.x, 2.0f) + std::pow(difference.y, 2.0f))); // In pixels
		const dVec3 direction{(from.x - to.x) / lineLength, (from.y - to.y) / lineLength, 0.0f};
		const dVec3 directionSecond{DCore::Math::Cross(direction, {0.0f, 0.0f, -1.0f})};
		const dVec3 directionThird{DCore::Math::Cross(direction, {0.0f, 0.0f, 1.0f})};
		const ImVec2 p1{DCore::Math::Lerp(from.x, to.x, arrowRatio), DCore::Math::Lerp(from.y, to.y, arrowRatio)};
		const ImVec2 p2{p1.x + direction.x * arrowSizePixels.y + directionSecond.x * arrowSizePixels.x / 2.0f, p1.y + direction.y * arrowSizePixels.y + directionSecond.y * arrowSizePixels.x / 2.0f};
		const ImVec2 p3{p1.x + direction.x * arrowSizePixels.y + directionThird.x * arrowSizePixels.x / 2.0f, p1.y + direction.y * arrowSizePixels.y + directionThird.y * arrowSizePixels.x / 2.0f};
		drawList->AddTriangleFilled(p1, p2, p3, color);
		std::array<dVec2, 3> points;
		points[0] = {p1.x, p1.y};
		points[1] = {p2.x, p2.y};
		points[2] = {p3.x, p3.y};
		return DCore::Math::IsPointInsideTriangle({io.MousePos.x, io.MousePos.y}, points);
	};
	if (m_isCreatingTransition)
	{
		drawTransition({m_fromStateCenterPos.x, m_fromStateCenterPos.y}, io.MousePos, false);
	}
	m_editorAnimationStateMachine.IterateOnStates
	(
		[&](stateConstRefType fromState) -> bool
		{
			const dVec2& fromStatePosition(m_editorAnimationStateMachine.GetPositionOfStateAtIndex(fromState.GetIndex()));
			const ImVec2 minFromStatePos(canvasCenterPos.x + (fromStatePosition.x - canvasCenterValue.x) * m_pixelsPerUnit, canvasCenterPos.y + (fromStatePosition.y - canvasCenterValue.y) * m_pixelsPerUnit);
			const ImVec2 fromStateCenterPos{minFromStatePos.x + stateSizePixels.x / 2.0f, minFromStatePos.y + stateSizePixels.y / 2.0f};
			m_editorAnimationStateMachine.IterateOnTransitionsOfStateAtIndex
			(
				fromState.GetIndex(), 
				[&](transitionConstRefType transition) -> bool
				{
					size_t toStateIndex(transition->GetToStateIndex());
					stateConstRefType toState(m_editorAnimationStateMachine.TryGetStateAtIndex(toStateIndex));
					DASSERT_E(toState.IsValid());
					const dVec2& toStatePosition(m_editorAnimationStateMachine.GetPositionOfStateAtIndex(toStateIndex));
					const ImVec2 minToStatePos(canvasCenterPos.x + (toStatePosition.x - canvasCenterValue.x) * m_pixelsPerUnit, canvasCenterPos.y + (toStatePosition.y - canvasCenterValue.y) * m_pixelsPerUnit);
					const ImVec2 toStateCenterPos{minToStatePos.x + stateSizePixels.x / 2.0f, minToStatePos.y + stateSizePixels.y / 2.0f};				
					bool isCursorHoveredTransition(drawTransition(fromStateCenterPos, toStateCenterPos, transition == m_selectedTransition));
					if (isCursorHoveredTransition && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					{
						m_selectedTransition = transition;
						InspectorPanel::Get().DisplayTransition(transition.GetIndex(), fromState.GetIndex(), s_animationStateMachinePanels.GetRefFromIndex(m_containerIndex));
					}
					if (isCursorHoveredTransition && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
					{
						ImGui::OpenPopup("TransitionActionMenuPopup");
						m_indexOfStateWithTransitionWithActionMenuOpened = fromState.GetIndex();
						m_indexOfTransitionWithActionMenuOpened = transition.GetIndex();
					}
					if (fromState.GetIndex() == m_indexOfStateWithTransitionWithActionMenuOpened && 
							transition.GetIndex() == m_indexOfTransitionWithActionMenuOpened && 
							ImGui::BeginPopup("TransitionActionMenuPopup"))
					{
						if (ImGui::Selectable("Delete transition"))
						{
							InspectorPanel::Get().Clear();
							m_editorAnimationStateMachine.DeleteTransition(fromState.GetIndex(), transition.GetIndex());
						}
						ImGui::EndPopup();
					}
					return false;
				}
			);
		return false;
		}
	);
	m_editorAnimationStateMachine.IterateOnStates
	(
		[&](stateConstRefType state) -> bool
		{
			const bool isStateSelected(m_selectedStatesMask[state.GetIndex()]);
			const dVec2& statePosition(m_editorAnimationStateMachine.GetPositionOfStateAtIndex(state.GetIndex()));
			const stringType& stateName(state->GetName());
			const ImVec2 textSize(ImGui::CalcTextSize(stateName.c_str()));
			const ImVec2 minStatePos(canvasCenterPos.x + (statePosition.x - canvasCenterValue.x) * m_pixelsPerUnit, canvasCenterPos.y + (statePosition.y - canvasCenterValue.y) * m_pixelsPerUnit);
			const ImVec2 maxStatePos(minStatePos.x + stateSizePixels.x, minStatePos.y + stateSizePixels.y);
			const auto drawBorder = [&]() -> void
			{
				const ImVec2 minBorderPos{minStatePos.x - stateBorderSizePixels.x, minStatePos.y - stateBorderSizePixels.y};
				const ImVec2 maxBorderPos{maxStatePos.x + stateBorderSizePixels.x, maxStatePos.y + stateBorderSizePixels.y};
				drawList->AddRectFilled(minBorderPos, maxBorderPos, IM_COL32(255, 255, 0, 255));
			};
			// State draw
			if (m_selectedStatesMask[state.GetIndex()])
			{
				drawBorder();
			}
			ImGui::SetCursorScreenPos(minStatePos);
			ImGui::InvisibleButton(state->GetName().c_str(), {stateSizePixels.x, stateSizePixels.y});
			const bool isHovered(ImGui::IsItemHovered());
			const bool isActive(ImGui::IsItemActive());
			const bool isStateRightClicked(ImGui::IsItemClicked(ImGuiMouseButton_Right));
			const bool isStateLeftClicked(ImGui::IsItemClicked(ImGuiMouseButton_Left));
			if (m_isCreatingTransition)
			{
				bool isDifferentFromFromState(m_fromStateIndex != state.GetIndex());
				if (!isDifferentFromFromState)
				{
					m_fromStateCenterPos = {minStatePos.x + stateSizePixels.x / 2.0f, minStatePos.y + stateSizePixels.y / 2.0f};
				}
				if (isHovered && isDifferentFromFromState)
				{
					drawBorder();
				}
				if (isStateLeftClicked && isDifferentFromFromState)
				{
					using createTransitionResult = EditorAnimationStateMachine::createTransitionResult;
					m_isCreatingTransition = false;
					switch (m_editorAnimationStateMachine.CreateTransition(m_fromStateIndex, state.GetIndex()))
					{
					case createTransitionResult::AlreadyExists:
						Log::Get().ConsoleLog(LogLevel::Error, "Transition from state \"%s\" to state \"%s\" is already created.", 
								m_editorAnimationStateMachine.TryGetStateAtIndex(m_fromStateIndex)->GetName().c_str(), 
								m_editorAnimationStateMachine.TryGetStateAtIndex(state.GetIndex())->GetName().c_str());
						break;
					default:
						break;
					}
				}
			}
			drawList->AddRectFilled(minStatePos, maxStatePos, state.GetIndex() == m_editorAnimationStateMachine.GetInitialStateIndex() ? IM_COL32(0, 0, 255, 255) : IM_COL32(0, 0, 0, 255));
			ImGui::SetCursorScreenPos({minStatePos.x + (stateSizePixels.x - textSize.x)/2.0f, minStatePos.y + (stateSizePixels.y - textSize.y)/2.0f});
			ImGui::Text("%s", stateName.c_str());
			if (m_isCreatingTransition)
			{
				return false;
			}
			//
			// Action menu
			if (isStateRightClicked)
			{
				ImGui::OpenPopup("StateActionMenuPopup");
				m_stateWithActionMenuOpened = state;
			}
			if (m_stateWithActionMenuOpened.IsValid() && m_stateWithActionMenuOpened == state)
			{
				if (ImGui::BeginPopup("StateActionMenuPopup"))
				{
					if (ImGui::Selectable("Create transition"))
					{
						m_isCreatingTransition = true;
						m_fromStateIndex = state.GetIndex();
						DeselectAllStates();
					}
					if (ImGui::Selectable("Delete state"))
					{
						if (state.GetIndex() == m_editorAnimationStateMachine.GetInitialStateIndex())
						{
							Log::Get().ConsoleLog(LogLevel::Error, "%s", "The initial state cannot be deleted.");
						}
						else
						{
							m_editorAnimationStateMachine.DeleteState(state.GetIndex());
						}
					}
					if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
					{
						ImGui::OpenPopup("RenamePopup");
						strcpy(m_stateName, state->GetName().c_str());
					}
					if (ImGui::BeginPopup("RenamePopup"))
					{
						const DrawStateActionResult result(DrawStateAction(StateAction::Rename, &state));
						if (result == DrawStateActionResult::Done || result == DrawStateActionResult::Canceled)
						{
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					if (ImGui::Selectable("Set initial state"))
					{
						m_editorAnimationStateMachine.SetInitialStateIndex(state.GetIndex());
					}
					ImGui::EndPopup();
				}
			}
			//
			// Movement
			if (isHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			{
				m_isDraggingState = true; 
				if (!io.KeyCtrl)
				{
					DeselectAllStates();
				}
				m_selectedStatesMask[state.GetIndex()] = true;
				m_editorAnimationStateMachine.IterateOnStates
				(
					[&](stateConstRefType stateToMove) -> bool
					{
						if (!m_selectedStatesMask[stateToMove.GetIndex()])
						{
							return false;
						}
						const dVec2 currentStatePosition(m_editorAnimationStateMachine.GetPositionOfStateAtIndex(stateToMove.GetIndex()));
						m_editorAnimationStateMachine.SetStatePosition(stateToMove.GetIndex(), {currentStatePosition.x + io.MouseDelta.x/m_pixelsPerUnit, currentStatePosition.y + io.MouseDelta.y/m_pixelsPerUnit});
						return false;
					}
				);
			}
			// Selection
			else if (isHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				if (m_isDraggingState)
				{
					m_isDraggingState = false;
				}
				else
				{
					if (io.KeyCtrl)
					{
						m_selectedStatesMask[state.GetIndex()] = !isStateSelected;
						// TODO. Clear inspector panel from states.
					}
					else
					{
						DeselectAllStates();
						m_selectedTransition.Invalidate();
						m_selectedStatesMask[state.GetIndex()] = true;
						InspectorPanel::Get().DisplayState(state, m_editorAnimationStateMachine);
					}
				}
			}
			//
			// TODO. Draw created transitions
			return false;
		}
	);
}

AnimationStateMachinePanel::DrawStateActionResult AnimationStateMachinePanel::DrawStateAction(StateAction stateAction, stateConstRefType* maybeState, const dVec2* maybeStatePosition)
{
	bool toPerformAction(false);
	ImGui::Text("Enter state name");
	if (ImGui::InputText("##StateName", m_stateName, sizeof m_stateName, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		toPerformAction = true;
	}
	if (m_isStateNameEmpty)
	{
		ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "%s", "A state name cannot be empty");
	}
	else if (m_isStateNameNotUnique)
	{
		ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "%s", "A state name must be unique");
	}
	if (ImGui::Button("Create"))
	{
		toPerformAction = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		std::memset(m_stateName, 0, stateNameSize);
		m_isStateNameEmpty = false;
		m_isStateNameNotUnique = false;
		return DrawStateActionResult::Canceled;
	}
	if (!toPerformAction)
	{
		return DrawStateActionResult::Working;
	}
	m_isStateNameEmpty = false;
	m_isStateNameNotUnique = false;
	switch (stateAction)
	{
	case StateAction::Create:
	{
		DASSERT_E(maybeStatePosition != nullptr);
		const dVec2& statePosition(*maybeStatePosition);
		stateConstRefType createdState;
		switch (m_editorAnimationStateMachine.CreateState(m_stateName, statePosition, &createdState))
		{
		case createStateResult::Ok:
			std::memset(m_stateName, 0, stateNameSize);
			if (createdState.GetIndex() >= m_selectedStatesMask.size())
			{
				m_selectedStatesMask.resize(createdState.GetIndex() + 1);
			}
			m_selectedStatesMask[createdState.GetIndex()] = false;
			return DrawStateActionResult::Done;
		case createStateResult::StateNameEmpty:
			m_isStateNameEmpty = true;
			return DrawStateActionResult::Working;
		case createStateResult::StateWithSameName:
			m_isStateNameNotUnique = true;
			return DrawStateActionResult::Working;
		}
		return DrawStateActionResult::Working;
	}
	case StateAction::Rename:
	{
		DASSERT_E(maybeState != nullptr);
		stateConstRefType state(*maybeState);
		switch (m_editorAnimationStateMachine.SetNameOfState(state.GetIndex(), m_stateName))
		{
		case renameStateResult::Ok:
			std::memset(m_stateName, 0, stateNameSize);
			return DrawStateActionResult::Done;
		case renameStateResult::StateNameEmpty:
			m_isStateNameEmpty = true;
			return DrawStateActionResult::Working;
		case renameStateResult::StateWithSameName:
			m_isStateNameNotUnique = true;
			return DrawStateActionResult::Working;
		}
		return DrawStateActionResult::Working;
	}
	}
	return DrawStateActionResult::Working;
}

void AnimationStateMachinePanel::DeselectAllStates()
{
	for (size_t index(0); index < m_selectedStatesMask.size(); index++)
	{
		m_selectedStatesMask[index] = false;
	}
}

void AnimationStateMachinePanel::DrawParameterNameEntry(std::function<void()> action)
{
	if (!ImGui::BeginPopup("ParameterActionMenuPopup"))
	{
		return; 
	}
	bool toPerformAction(false);
	ImGui::Text("Enter parameter name");
	if (ImGui::InputText("##ParamaterName", m_parameterName, sizeof m_parameterName , ImGuiInputTextFlags_EnterReturnsTrue))
	{
		toPerformAction = true;
	}
	if (m_isParameterNameEmpty)
	{
		ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "%s", "A parameter name cannot be empty");
	}
	else if (m_isParameterNameNotUnique)
	{
		ImGui::TextColored({1.0f, 0.0f, 0.0f, 1.0f}, "%s", "A parameter name must be unique");
	}
	if (ImGui::Button("Create"))
	{
		toPerformAction = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		m_isParameterNameEmpty = false;
		m_isParameterNameNotUnique = false;
		std::memset(m_parameterName, 0, parameterNameSize);
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return; 
	}
	if (!toPerformAction)
	{
		ImGui::EndPopup();
		return;
	}
	m_isParameterNameEmpty = false;
	m_isParameterNameNotUnique = false;
	action();
	if (!m_isParameterNameEmpty && !m_isParameterNameNotUnique)
	{
		std::memset(m_parameterName, 0, parameterNameSize);
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}

void AnimationStateMachinePanel::CreateParameter()
{
	using createParameterResult = EditorAnimationStateMachine::createParameterResult;
	using integerParameterConstRefType = DCore::integerParameterConstRefType;
	using floatParameterConstRefType = DCore::floatParameterConstRefType;
	using logicParameterConstRefType = DCore::logicParameterConstRefType;
	using triggerParameterConstRefType = DCore::triggerParameterConstRefType;
	createParameterResult result(createParameterResult::Ok);
	ParameterInfo createdParameterInfo;
	switch (m_parameterTypeToAdd) 
	{
	case parameterType::Integer:
	{
		integerParameterConstRefType parameter(m_editorAnimationStateMachine.MakeInvalidConstRefToParameter<integerParameterConstRefType>());
		result = m_editorAnimationStateMachine.CreateParameter<parameterType::Integer>(m_parameterName, parameter);
		createdParameterInfo = {m_parameterTypeToAdd, parameter.GetIndex()};
		break;
	}
	case parameterType::Float:
	{
		floatParameterConstRefType parameter(m_editorAnimationStateMachine.MakeInvalidConstRefToParameter<floatParameterConstRefType>());
		result = m_editorAnimationStateMachine.CreateParameter<parameterType::Float>(m_parameterName, parameter);
		createdParameterInfo = {m_parameterTypeToAdd, parameter.GetIndex()};
		break;
	}
	case parameterType::Logic:
	{
		logicParameterConstRefType parameter(m_editorAnimationStateMachine.MakeInvalidConstRefToParameter<logicParameterConstRefType>());
		result = m_editorAnimationStateMachine.CreateParameter<parameterType::Logic>(m_parameterName, parameter);
		createdParameterInfo = {m_parameterTypeToAdd, parameter.GetIndex()};
		break;
	}
	case parameterType::Trigger:
	{
		triggerParameterConstRefType parameter(m_editorAnimationStateMachine.MakeInvalidConstRefToParameter<triggerParameterConstRefType>());
		result = m_editorAnimationStateMachine.CreateParameter<parameterType::Trigger>(m_parameterName, parameter);
		createdParameterInfo = {m_parameterTypeToAdd, parameter.GetIndex()};
		break;
	}
	}
	switch (result) 
	{
	case createParameterResult::ParameterNameEmpty:
		m_isParameterNameEmpty = true;
		return;
	case createParameterResult::ParameterWithSameName:
		m_isParameterNameNotUnique = true;
		return;
	default:
		m_parameterInfos.push_back(createdParameterInfo);
		return;
	}
}

void AnimationStateMachinePanel::UpdateSimulation()
{
	if (!m_isPlaying)
	{
		return;
	}
	m_editorAnimationStateMachine.Tick(ImGui::GetIO().DeltaTime);
}

}
