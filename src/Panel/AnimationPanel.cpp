#include "AnimationPanel.h"
#include "AnimationManager.h"
#include "Log.h"
#include "Draw.h"

#include "glm/ext/scalar_constants.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <climits>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <sstream>
#include <stack>



namespace DEditor
{

AnimationPanel::animationPanelContainerType AnimationPanel::s_openedAnimationPanels;

KeyframeInfo::KeyframeInfo(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t keyframeId)
	:
	ComponentId(componentId),
	AttributeId(attributeId),
	AttributeComponentId(attributeComponentId),
	KeyframeId(keyframeId)
{}

KeyframeInfo::KeyframeInfo(const KeyframeInfo& other)
	:
	ComponentId(other.ComponentId),
	AttributeId(other.AttributeId),
	AttributeComponentId(other.AttributeComponentId),
	KeyframeId(other.KeyframeId)
{}

AnimationPanel::AnimationPanel(const char* panelName, Animation&& editorAnimation)
	:
	m_panelName(panelName),
	m_editorAnimation(std::move(editorAnimation)),
	m_coreAnimation(AnimationManager::Get().LoadCoreAnimation(m_editorAnimation.GetUUID())),
	m_isOpened(true),
	m_stepSize(1.0f),
	m_pixelsPerUnit(128.0f),
	m_scrolling(0, 0),
	m_stepSizeOrder(0),
	m_numberOfSelectedAttributesComponents(0),
	m_cursorTime(0.0f),
	m_isBoxSelectionActive(false),
	m_isPlaying(false)
{}

AnimationPanel::AnimationPanel(AnimationPanel&& other) noexcept
	:
	m_panelName(std::move(other.m_panelName)),
	m_editorAnimation(std::move(other.m_editorAnimation)),
	m_coreAnimation(other.m_coreAnimation),
	m_isOpened(other.m_isOpened),
	m_stepSize(other.m_stepSize),
	m_pixelsPerUnit(other.m_pixelsPerUnit),
	m_numberOfSelectedAttributesComponents(other.m_numberOfSelectedAttributesComponents),
	m_scrolling(other.m_scrolling),
	m_stepSizeOrder(other.m_stepSizeOrder),
	m_cursorTime(other.m_cursorTime),
	m_isBoxSelectionActive(false),
	m_attachedEntity(other.m_attachedEntity),
	m_isPlaying(false)
{
	other.m_coreAnimation.Invalidate();
}

void AnimationPanel::OpenAnimationPanel(const char* animationName, Animation&& editorAnimation)
{
	DASSERT_E(!IsPanelWithAnimationUUIDOpened(editorAnimation.GetUUID()));
	s_openedAnimationPanels.PushBack(animationName, std::move(editorAnimation));
}
	
bool AnimationPanel::IsPanelWithAnimationUUIDOpened(const DCore::UUIDType& uuid)
{
	bool returnValue(false);
	s_openedAnimationPanels.Iterate
	(
		[&](animationPanelContainerType::ConstRef animationPanel) -> bool
		{
			if (animationPanel->GetAnimationUUID() == uuid)
			{
				returnValue = true;
				return true;
			}
			return false;
		}
	);
	return returnValue;
}

void AnimationPanel::RenderAnimationPanels()
{
	s_openedAnimationPanels.Iterate
	(
		[&](animationPanelContainerType::Ref animationPanel) -> bool
		{
			if (animationPanel->Render())
			{
				{
					DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
					animationPanel->m_coreAnimation.Unload();
				}
				animationPanel->RemoveFromAllSynchronizationPanels();
				s_openedAnimationPanels.Remove(animationPanel);
			}
			return false;
		}
	);
}

bool AnimationPanel::Render()
{
	ImGuiWindowFlags windowFlags(0);
	if (!ImGui::Begin(m_panelName.c_str(), &m_isOpened, windowFlags))
	{
		ImGui::End();
		return false;
	}
	if (!m_isOpened)
	{
		ImGui::End();
		return true;
	}
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
		if (!m_coreAnimation.IsValid())
		{
			ImGui::End();
			return true;
		}
	}
	const ImVec2 regionAvail(ImGui::GetContentRegionAvail());
	if (ImGui::BeginChild("Config", {regionAvail.x * 0.4f, regionAvail.y}, ImGuiChildFlags_ResizeX | ImGuiChildFlags_Border))
	{
		DrawConfig();
		DrawMetachannels();
		DrawPlay();		
		DrawSynchronization();
	}
	ImGui::EndChild();
	ImGui::SameLine();
	if (ImGui::BeginChild("Canvas", ImGui::GetContentRegionAvail(), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		DrawCanvas();
	}
	ImGui::EndChild();
	ImGui::End();
	return false;
}

void AnimationPanel::AddSynchronizationPanel(size_t animationPanelIndex)
{
	animationPanelContainerType::Ref animationPanel(s_openedAnimationPanels.GetRefFromIndex(animationPanelIndex));
	DASSERT_E(animationPanel.IsValid());
	m_synchronizationPanels.insert({ animationPanel->GetAnimationUUID(), animationPanelIndex });
}

void AnimationPanel::RemoveSynchronizationPanel(size_t animationPanelIndex)
{
	animationPanelContainerType::Ref animationPanel(s_openedAnimationPanels.GetRefFromIndex(animationPanelIndex));
	DASSERT_E(animationPanel.IsValid());
	m_synchronizationPanels.erase(animationPanel->GetAnimationUUID());
}

void AnimationPanel::RemoveFromAllSynchronizationPanels()
{
	size_t selfAnimationPanelIndex(GetRefIndex());
	for (const auto& it : m_synchronizationPanels)
	{
		animationPanelContainerType::Ref animationPanel(s_openedAnimationPanels.GetRefFromIndex(it.second));
		DASSERT_E(animationPanel.IsValid());
		animationPanel->RemoveSynchronizationPanel(selfAnimationPanelIndex);
	}
}

void AnimationPanel::SetCursorTime(float time)
{
	m_cursorTime = time;
	DCore::ReadWriteLockGuard animationGuard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
	DCore::AnimationSimulator::Simulate(m_attachedEntity, m_coreAnimation, m_cursorTime, 0.0f, [](DCore::EntityRef, size_t){});
}

void AnimationPanel::AddIntegerKeyframe(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, const IntegerKeyframe& keyframe)
{
	KeyframeAddResult result(m_editorAnimation.TryAddIntegerKeyframe(componentId, attributeId, attributeComponentId, keyframe));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	const size_t numberOfAttributeComponents(componentForm.SerializedAttributes[attributeId].GetNumberOfAttributeComponents());
	switch (result)
	{
	case KeyframeAddResult::KeyframeAtTimeZero:
		Log::Get().ConsoleLog(LogLevel::Error, "%s", "Adding a keyframe at time 0.0 is forbidden.");
		break;
	case KeyframeAddResult::AlreadyAddedAtTime:
	{
		const DCore::AttributeName& attributeName(componentForm.SerializedAttributes[attributeId].GetAttributeName());
		if (numberOfAttributeComponents > 1)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "Cannot add a keyframe in the animation of \"%s\" (of component \"%s\" and attribute \"%s\"). There is already a keyframe at time %f", attributeName.GetComponentAtIndex(attributeComponentId).c_str(), componentForm.Name.c_str(), attributeName.GetName().c_str(), m_cursorTime);
			break;
		}
		Log::Get().ConsoleLog(LogLevel::Error, "Cannot add a keyframe in the animation of \"%s\" (of component \"%s\"). There is already a keyframe at time %f", attributeName.GetName().c_str(), componentForm.Name.c_str(), m_cursorTime);
		break;
	}
	case KeyframeAddResult::Ok:
		break;
	}
}

void AnimationPanel::AddFloatKeyframe(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, const FloatKeyframe& keyframe)
{
	const KeyframeAddResult result(m_editorAnimation.TryAddFloatKeyframe(componentId, attributeId, attributeComponentId, keyframe));
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	const size_t numberOfAttributeComponents(componentForm.SerializedAttributes[attributeId].GetNumberOfAttributeComponents());
	switch (result)
	{
	case KeyframeAddResult::KeyframeAtTimeZero:
		Log::Get().ConsoleLog(LogLevel::Error, "%s", "Adding a keyframe at time 0.0 is forbidden.");
		break;
	case KeyframeAddResult::AlreadyAddedAtTime:
	{
		const DCore::AttributeName& attributeName(componentForm.SerializedAttributes[attributeId].GetAttributeName());
		if (numberOfAttributeComponents > 1)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "Cannot add a keyframe in the animation of \"%s\" (of component \"%s\" and attribute \"%s\"). There is already a keyframe at time %f", attributeName.GetComponentAtIndex(attributeComponentId).c_str(), componentForm.Name.c_str(), attributeName.GetName().c_str(), m_cursorTime);
			break;
		}
		Log::Get().ConsoleLog(LogLevel::Error, "Cannot add a keyframe in the animation of \"%s\" (of component \"%s\"). There is already a keyframe at time %f", attributeName.GetName().c_str(), componentForm.Name.c_str(), m_cursorTime);
		break;
	}
	case KeyframeAddResult::Ok:
		break;
	}
}

void AnimationPanel::DrawCanvas()
{
	ImGuiIO& io(ImGui::GetIO());
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	ImVec2 canvasSize(ImGui::GetContentRegionAvail());
	ImVec2 canvasMin(ImGui::GetCursorScreenPos());
	ImVec2 canvasMax(canvasMin.x + canvasSize.x, canvasMin.y + canvasSize.y);
	drawList->AddRectFilled(canvasMin, canvasMax, IM_COL32(50, 50, 50, 255));
	ImGui::SetNextItemAllowOverlap();
	ImGui::InvisibleButton("CanvasButton", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	bool isActive(ImGui::IsItemActive());
	bool isHovered(ImGui::IsItemHovered());
	ImVec2 canvasCenterPos(canvasMin.x + canvasSize.x/2.0f, canvasMin.y + canvasSize.y/2.0f);
	if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
	{
		m_scrolling.x += io.MouseDelta.x;
		m_scrolling.y += io.MouseDelta.y;
	}
	if (isHovered && ImGui::IsKeyPressed(ImGuiKey_C))
	{
		ClearKeyframesFromSelection();
	}
	if (isHovered && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		DeleteSelectedKeyframes();
	}
	// Zoom
	if (isHovered && io.MouseWheel != 0.0f)
	{
		float currentCenterValueX(-m_scrolling.x/m_pixelsPerUnit);
		float currentCenterValueY(-m_scrolling.y/m_pixelsPerUnit);
		float currentMouseValueX(currentCenterValueX + (io.MousePos.x - canvasCenterPos.x)/m_pixelsPerUnit);
		float currentMouseValueY(currentCenterValueY + (io.MousePos.y - canvasCenterPos.y)/m_pixelsPerUnit);
		m_pixelsPerUnit *= io.MouseWheel == 1.0f ? 1.2f : 0.83333f;
		if (m_stepSize * m_pixelsPerUnit < 80.0f)
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
		else if (m_stepSize * m_pixelsPerUnit > 256.0f)
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
	drawList->PushClipRect(canvasMin, canvasMax, true);
	float centerValueX(-m_scrolling.x/m_pixelsPerUnit);
	float centerValueY(m_scrolling.y/m_pixelsPerUnit);
	// Debug
	//float currentMouseValueX(centerValueX + (io.MousePos.x - canvasCenterPos.x)/m_pixelsPerUnit);
	//float currentMouseValueY(centerValueY + (io.MousePos.y - canvasCenterPos.y)/m_pixelsPerUnit);
	//Log::Get().TerminalLog("%f, %f", currentMouseValueX, currentMouseValueY);
	//
	float leftValue(centerValueX + (canvasMin.x - canvasCenterPos.x)/m_pixelsPerUnit);	
	int nx(glm::floor(leftValue/m_stepSize));
	float valueX(nx * m_stepSize);
	float x((valueX - leftValue) * m_pixelsPerUnit);
	float topValue(centerValueY - (canvasMin.y - canvasCenterPos.y)/m_pixelsPerUnit);
	int ny(glm::floor(topValue/m_stepSize));
	float valueY(ny * m_stepSize);
	float y((topValue - valueY) * m_pixelsPerUnit);
	for (float yi(y); yi < canvasSize.y; yi += m_stepSize * m_pixelsPerUnit)
	{
		constexpr ImVec2 valueTextPadding{5.0f, 2.0f};
		char buff[16];
		sprintf(buff, "%f", valueY);
		drawList->AddLine({canvasMin.x, canvasMin.y + yi}, {canvasMax.x, canvasMin.y + yi}, IM_COL32(255, 255, 255, 20));
		drawList->AddText({canvasMin.x + valueTextPadding.x, canvasMin.y + yi + valueTextPadding.y}, IM_COL32(255, 255, 255, 255), buff);
		valueY -= m_stepSize;
	}
	for (float xi(x); xi < canvasSize.x; xi += m_stepSize * m_pixelsPerUnit)
	{
		drawList->AddLine({canvasMin.x + xi, canvasMin.y}, {canvasMin.x + xi, canvasMax.y}, IM_COL32(255, 255, 255, 20));
	}
	DrawKeyframes({canvasCenterPos.x, canvasCenterPos.y});
	// Handle box selection
	if (isHovered)
	{
		if (isActive && ImGui::IsKeyDown(ImGuiKey_MouseLeft))
		{
			if (!m_isBoxSelectionActive)
			{
				m_boxSelectionInitialPos = {io.MousePos.x, io.MousePos.y};
				m_isBoxSelectionActive = true;
			}
			else
			{
				drawList->AddRect({m_boxSelectionInitialPos.x, m_boxSelectionInitialPos.y}, io.MousePos, IM_COL32(255, 255, 255, 255));
			}
		}
		else if (m_isBoxSelectionActive && ImGui::IsKeyReleased(ImGuiKey_MouseLeft))
		{
			// Select keyframes inside the box.
			HandleKeyframeBoxSelection({canvasCenterPos.x, canvasCenterPos.y});
			m_isBoxSelectionActive = false;
		}
	}
	else
	{
		m_isBoxSelectionActive = false;
	}
	drawList->PopClipRect();
	// Draw timeline
	constexpr float timelineSizeY{30.0f};
	const ImVec2 timelineMaxPosY{canvasMax.x, canvasMin.y + timelineSizeY};
	drawList->PushClipRect(canvasMin, timelineMaxPosY, true);
	drawList->AddRectFilled(canvasMin, timelineMaxPosY, IM_COL32_BLACK);
	for (float xi(x); xi < canvasSize.x; xi += m_stepSize * m_pixelsPerUnit)
	{
		constexpr ImVec2 valueTextPadding{5.0f, -5.0f};
		if (valueX >= 0.0f && valueX <= m_editorAnimation.GetDuration())
		{
			char buff[16];
			sprintf(buff, "%f", valueX);
			drawList->AddLine({canvasMin.x + xi, canvasMin.y + timelineSizeY * 0.7f}, {canvasMin.x + xi, canvasMin.y + timelineSizeY}, IM_COL32(255, 255, 255, 255));
			drawList->AddText({canvasMin.x + xi + valueTextPadding.x, canvasMin.y + timelineSizeY * 0.7f + valueTextPadding.y}, IM_COL32(255, 255, 255, 255), buff);
		}
		valueX += m_stepSize;
	}
	// Draw metachannels;
	constexpr float metachannelCursorHeight{0.2f * timelineSizeY};		
	m_editorAnimation.IterateOnMetachannels
	(
		[&](auto metachannel) -> bool
		{
			constexpr float cursorWidth{7.5f};
			constexpr float cursorHalfWidth{cursorWidth / 2.0f};
			const ImVec2 cursorHeadPos {
				canvasCenterPos.x + (metachannel->GetTime() - centerValueX) * m_pixelsPerUnit,
				canvasMin.y + metachannelCursorHeight
			};
			const ImVec2 cursorLeftPos{cursorHeadPos.x - cursorHalfWidth, canvasMin.y};
			const ImVec2 cursorRightPos{cursorHeadPos.x + cursorHalfWidth, canvasMin.y};
			drawList->AddTriangleFilled(cursorHeadPos, cursorLeftPos, cursorRightPos, IM_COL32(255, 255, 255, 255));
			return false;
		}
	);
	// Draw cursor
	constexpr float cursorWidth{15.0f};
	constexpr float cursorHalfWidth{cursorWidth / 2.0f};
	constexpr float cursorHeight{0.4f * timelineSizeY};
	const float cursorHeadPosX(canvasCenterPos.x + (m_cursorTime - centerValueX) * m_pixelsPerUnit);
	const ImVec2 cursorHeadPos{cursorHeadPosX, canvasMin.y + cursorHeight};
	const ImVec2 cursorLeftPos{cursorHeadPosX - cursorHalfWidth, canvasMin.y};
	const ImVec2 cursorRightPos{cursorHeadPosX + cursorHalfWidth, canvasMin.y};
	drawList->AddTriangleFilled(cursorHeadPos, cursorLeftPos, cursorRightPos, IM_COL32(255, 0, 0, 255));
	drawList->PopClipRect();
	// Cursor line
	drawList->AddLine(cursorHeadPos, {cursorHeadPos.x, canvasMax.y}, IM_COL32(255, 0, 0, 255));
	// Cursor movement
	ImGui::SetCursorScreenPos(canvasMin);
	ImGui::SetNextItemAllowOverlap();
	if (ImGui::InvisibleButton("Timeline", {canvasMax.x - canvasMin.x, timelineSizeY}))
	{
		m_cursorTime = centerValueX + (io.MousePos.x - canvasCenterPos.x)/m_pixelsPerUnit;
		m_cursorTime = std::max(0.0f, std::min(m_cursorTime, m_editorAnimation.GetDuration()));
		DCore::ReadWriteLockGuard animationGuard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
		DCore::AnimationSimulator::Simulate(m_attachedEntity, m_coreAnimation, m_cursorTime, 0.0f, [](DCore::EntityRef, size_t){});
		SetSynchronizedCursorTime(m_cursorTime);
	}
	ImGui::SetCursorScreenPos(cursorLeftPos);
	ImGui::InvisibleButton("Cursor", {cursorWidth, cursorHeight});
	if (ImGui::IsItemActive())
	{
		m_cursorTime = centerValueX + (io.MousePos.x - canvasCenterPos.x)/m_pixelsPerUnit;
		m_cursorTime = std::max(0.0f, std::min(m_cursorTime, m_editorAnimation.GetDuration()));
		DCore::ReadWriteLockGuard animationGuard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
		DCore::AnimationSimulator::Simulate(m_attachedEntity, m_coreAnimation, m_cursorTime, 0.0f, [](DCore::EntityRef, size_t){});
		SetSynchronizedCursorTime(m_cursorTime);
	}
}

void AnimationPanel::DrawConfig()
{	
	if (ImGui::Button("Save"))
	{
		AnimationManager::Get().SaveChanges(m_editorAnimation);
		m_coreAnimation = m_editorAnimation.GenerateCoreAnimation();
	}
	float animationDuration(m_editorAnimation.GetDuration());
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Duration");
	ImGui::SameLine();
	if (ImGui::DragFloat("##Duration", &animationDuration, 0.1f))
	{
		if (m_synchronizationPanels.empty())
		{
			animationDuration = std::max(animationDuration, 0.0f);
			m_editorAnimation.SetDuration(animationDuration);
		}
		else
		{
			Log::Get().TerminalLog("%s", "Cannot change animation duration while synchronized.");
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot change animation duration while synchronized.");
		}
	}
	if (ImGui::Button("+"))
	{
		if (m_numberOfSelectedAttributesComponents == 0)
		{
			Log::Get().ConsoleLog(LogLevel::Error, "%s", "There is no attribute selected. Can't add keyframe.");
		}
		else
		{
			m_editorAnimation.IterateOnComponentIdsAndAttributeIds
			(
				[&](DCore::ComponentIdType componentId, const std::vector<DCore::AttributeIdType>& attributeIds) -> bool
				{
					for (DCore::AttributeIdType attributeId : attributeIds)
					{
						const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
						const DCore::AttributeType attributeType(componentForm.SerializedAttributes[attributeId].GetAttributeType());
						const DCore::AttributeKeyframeType attributeKeyframeType(componentForm.SerializedAttributes[attributeId].GetKeyframeType());
						const size_t numberOfAttributeComponents(componentForm.SerializedAttributes[attributeId].GetNumberOfAttributeComponents());
						for (size_t attributeComponentId(0); attributeComponentId < numberOfAttributeComponents; attributeComponentId++)
						{
							if (!IsAttributeOfComponentSelected(componentId, attributeId, attributeComponentId))
							{
								continue;
							}
							if (attributeKeyframeType == DCore::AttributeKeyframeType::Integer)
							{
								AddIntegerKeyframe(componentId, attributeId, attributeComponentId, IntegerKeyframe(m_cursorTime, 0));
							}
							else 
							{
								const std::vector<FloatKeyframe>& keyframes(m_editorAnimation.GetFloatKeyframes(componentId, attributeId, attributeComponentId));
								if (keyframes.size() == 1)
								{
									AddFloatKeyframe(componentId, attributeId, attributeComponentId, FloatKeyframe(m_cursorTime, keyframes[0].GetMainPoint().y));
								}
								else
								{
									if (m_cursorTime > keyframes.back().GetMainPoint().x)
									{
										AddFloatKeyframe(componentId, attributeId, attributeComponentId, FloatKeyframe(m_cursorTime, keyframes.back().GetMainPoint().y));
									}
									else
									{
										size_t nextKeyframeIndex(1);
										for (const FloatKeyframe& keyframe : keyframes)
										{
											if (m_cursorTime > keyframes[nextKeyframeIndex].GetMainPoint().x)
											{
												nextKeyframeIndex++;
												continue;
											}
											const FloatKeyframe& nextKeyframe(keyframes[nextKeyframeIndex]);
											const DCore::DVec2 p1(keyframe.GetMainPoint());
											const DCore::DVec2 p2(keyframe.GetRightControlPoint());
											const DCore::DVec2 p3(nextKeyframe.GetLeftControlPoint());
											const DCore::DVec2 p4(nextKeyframe.GetMainPoint());
											const float t((m_cursorTime - p1.x)/(p4.x - p1.x));
											const float newKeyframeValue(pow(1-t, 3)*p1.y + 3*t*pow(1-t, 2)*p2.y + 3*pow(t, 2)*(1-t)*p3.y + pow(t, 3)*p4.y);
											AddFloatKeyframe(componentId, attributeId, attributeComponentId, FloatKeyframe(m_cursorTime, newKeyframeValue));
											break;
										}
									}
								}
							}
						}
					}
					return false;
				}
			);
		}
	}
	m_editorAnimation.IterateOnComponentIdsAndAttributeIds([&](DCore::ComponentIdType componentId, const std::vector<DCore::AttributeIdType>& attributes) -> bool { return DrawComponentAndAttributes(componentId, attributes); });
	if (ImGui::Button("Add Attribute"))
	{
		ImGui::OpenPopup("AddAttributePopup");
	}
	if (ImGui::BeginPopup("AddAttributePopup"))
	{
		const std::vector<DCore::ComponentForm>& componentForms(DCore::ComponentForms::Get().GetComponentForms());
		for (const DCore::ComponentForm& componentForm : componentForms)
		{
			if (componentForm.Name == "Children Component" ||
				componentForm.Name == "Child Component")
			{
				continue;
			}
			if (componentForm.SerializedAttributes.size() == 0)
			{
				continue;
			}
			std::vector<DCore::AttributeIdType> notAddedAttributes;
			for (const DCore::SerializedAttribute& attribute : componentForm.SerializedAttributes)
			{
				const DCore::AttributeKeyframeType attributeKeyframeType(attribute.GetKeyframeType());
				if (attributeKeyframeType == DCore::AttributeKeyframeType::NotDefined)
				{
					continue;
				}
				if (m_editorAnimation.IsAttributeOfComponentAdded(componentForm.Id, attribute.GetAttributeId()))
				{
					continue;
				}
				notAddedAttributes.push_back(attribute.GetAttributeId());
			}
			if (notAddedAttributes.size() == 0)
			{
				continue;
			}
			if (!ImGui::TreeNode(componentForm.Name.c_str()))
			{
				continue;
			}
			for (DCore::AttributeIdType attributeId : notAddedAttributes)
			{
				ImGui::Indent();
				if (ImGui::Selectable(componentForm.SerializedAttributes[attributeId].GetAttributeName().GetName().c_str()))
				{
					m_editorAnimation.MakeAnimation(componentForm.Id, attributeId);
				}
				ImGui::Unindent();
			}
			ImGui::TreePop();
		}
		ImGui::EndPopup();
	}
}

void AnimationPanel::DrawMetachannels()
{
	if (!ImGui::CollapsingHeader("Metachannels"))
	{
		return;
	}
	if (ImGui::Button("Add"))
	{
		m_editorAnimation.AddMetachannel();
	}
	size_t index(0);
	m_editorAnimation.IterateOnMetachannels
	(
		[&](auto metachannel) -> bool
		{
			std::stringstream buffStream;
			buffStream << "Matachannel " << index++;
			bool isOpened(ImGui::TreeNodeEx(buffStream.str().c_str(), ImGuiTreeNodeFlags_SpanAvailWidth));
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::Selectable("Remove"))
				{
					m_editorAnimation.RemoveMetachannelAtIndex(metachannel.GetIndex());
					ImGui::EndPopup();
					if (isOpened)
					{
						ImGui::TreePop();
					}
					return false;
				}
				ImGui::EndPopup();
			}
			if (!isOpened)
			{
				return false;
			}
			ImGui::AlignTextToFramePadding();
			ImGui::Text("ID");
			ImGui::SameLine();
			int metachannelId(static_cast<int>(metachannel->GetId()));
			if (ImGui::DragInt("##ID", &metachannelId, 1.0f, 0, INT_MAX))
			{
				metachannel->SetId(static_cast<size_t>(metachannelId));
			}
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Time");
			ImGui::SameLine();
			float metachannelTime(metachannel->GetTime());
			if (ImGui::DragFloat("##Time", &metachannelTime, 0.1f, 0.0f, m_editorAnimation.GetDuration()))
			{
				metachannelTime = std::min(metachannelTime, m_editorAnimation.GetDuration());
				metachannel->SetTime(metachannelTime);
			}
			ImGui::TreePop();
			return false;
		}
	);
}

void AnimationPanel::DrawPlay()
{
	using sceneRefType = DCore::SceneRef;
	using entityType = DCore::Entity;
	ImGuiIO& io(ImGui::GetIO());
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	if (!ImGui::CollapsingHeader("Simulation"))
	{
		return;
	}
	if (ImGui::Button("Attach"))
	{
		ImGui::OpenPopup("SelectEntityPopup");
	}
	if (ImGui::BeginPopup("SelectEntityPopup"))
	{
		DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
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
						DCore::DString name;
						entityRef.GetName(name);
						labelStream << name.Data();
						if (ImGui::Selectable(labelStream.str().c_str()))
						{
							m_attachedEntity = entityRef;
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
	DCore::ReadWriteLockGuard guard(DCore::LockType::ReadLock, *static_cast<DCore::SceneAssetManager*>(&DCore::AssetManager::Get()));
	if (m_attachedEntity.IsValid())
	{
		ImGui::SameLine();
		if (ImGui::Button("Detach"))
		{
			m_attachedEntity.Invalidate();
			m_isPlaying = false;
		}
		else
		{
			DCore::DString name;
			m_attachedEntity.GetName(name);
			ImGui::Text("Attachment %s", name.Data());
			if (ImGui::Button("Add Keyframes From Current Values"))
			{
				m_editorAnimation.IterateOnComponentIdsAndAttributeIds
				(
					[&](DCore::ComponentIdType componentId, const std::vector<DCore::AttributeIdType>& attributes) -> bool
					{
						const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
						for (const DCore::SerializedAttribute& attribute : componentForm.SerializedAttributes)
						{
							for (const DCore::AttributeIdType attributeId : attributes)
							{
								if (attributeId != attribute.GetAttributeId())
								{
									continue;
								}
								for (size_t attributeComponentId(0); attributeComponentId < attribute.GetNumberOfAttributeComponents(); attributeComponentId++)
								{
									if (!IsAttributeOfComponentSelected(componentId, attributeId, attributeComponentId))
									{
										continue;
									}
									DCore::ComponentRef<DCore::Component> component(m_attachedEntity.GetComponent(componentId));
									if (!component.IsValid())
									{
										continue;
									}
									const size_t attributeSizeBytes(attribute.GetAttributeSizeBytes());
									const size_t attributeComponentSizeBytes(attribute.GetAttributeComponentSizeBytes());
									char* attributeBytes(new char[attributeSizeBytes]);
									component.GetAttributePtr(attributeId, attributeBytes, attributeSizeBytes);
									switch (attribute.GetKeyframeType())
									{
									case DCore::AttributeKeyframeType::Integer:
									{
										DCore::DInt attributeComponent(0);
										std::memcpy(&attributeComponent, attributeBytes + attributeComponentId * attributeComponentSizeBytes, sizeof(DCore::DInt));
										if (m_cursorTime == 0.0f)
										{
											m_editorAnimation.SetIntegerKeyframeAtIndex(componentId, attributeId, attributeComponentId, 0, IntegerKeyframe(0.0f, attributeComponent));
											break;
										}
										AddIntegerKeyframe(componentId, attributeId, attributeComponentId, IntegerKeyframe(m_cursorTime, attributeComponent));
										break;
									}
									case DCore::AttributeKeyframeType::Float:
									{
										DCore::DFloat attributeComponent(0);
										std::memcpy(&attributeComponent, attributeBytes + attributeComponentId * attributeComponentSizeBytes, sizeof(DCore::DFloat));
										if (m_cursorTime == 0.0f)
										{
											m_editorAnimation.SetFloatKeyframeAtIndex(componentId, attributeId, attributeComponentId, 0, FloatKeyframe(0.0f, attributeComponent));
											break;
										}
										AddFloatKeyframe(componentId, attributeId, attributeComponentId, FloatKeyframe(m_cursorTime, attributeComponent));
										break;
									}
									default:
										break;
									}
									delete[] attributeBytes;
								}
							}
						}
						return false;
					}
				);
			}
		}
		if (!m_isPlaying)
		{
			static constexpr ImVec2 playSymbolSizes{80.0f, 50.0f};
			const ImVec2 cursorPos(ImGui::GetCursorScreenPos());
			Draw::DrawPlaySymbol({playSymbolSizes.x, playSymbolSizes.y}, {cursorPos.x, cursorPos.y});
			if (ImGui::InvisibleButton("PlayButton", playSymbolSizes))
			{
				m_isPlaying = true;
			}
		} 
		else
		{
			static constexpr ImVec2 stopSymbolSizes{15.0f, 50.0f};
			static constexpr float stopSymbolDistance{10.0f};
			ImVec2 cursorPos(ImGui::GetCursorScreenPos());
			if (ImGui::InvisibleButton("Stop", {2 * stopSymbolSizes.x + stopSymbolDistance, stopSymbolSizes.y}))
			{
				m_isPlaying = false;
			}
			Draw::DrawStopSymbol({stopSymbolSizes.x, stopSymbolSizes.y}, stopSymbolDistance, {cursorPos.x, cursorPos.y});
			if (m_cursorTime >= m_editorAnimation.GetDuration())
			{
				m_cursorTime = 0;
			}
			DCore::ReadWriteLockGuard animationGuard(DCore::LockType::ReadLock, *static_cast<DCore::AnimationAssetManager*>(&DCore::AssetManager::Get()));
			DCore::AnimationSimulator::Simulate(m_attachedEntity, m_coreAnimation, m_cursorTime, 0.0f, [](DCore::EntityRef, size_t){});
			m_cursorTime += io.DeltaTime;
		}
	}
	else
	{
		ImGui::Text("%s", "Attachment: NOT ATTACHED");
		m_isPlaying = false;
	}
//ImGui::TreePop();
}

void AnimationPanel::DrawSynchronization()
{
	if (!ImGui::CollapsingHeader("Synchronization"))
	{
		return;
	}
	if (ImGui::Button("Synchronize"))
	{
		ImGui::OpenPopup("SyncPopup");
	}
	if (ImGui::BeginPopup("SyncPopup"))
	{
		s_openedAnimationPanels.Iterate
		(
			[&](animationPanelContainerType::Ref animationPanel) -> bool
			{
				const DCore::UUIDType& animationUUID(animationPanel->GetAnimationUUID());
				if (animationPanel->GetAnimationUUID() == GetAnimationUUID())
				{
					return false;
				}
				if (m_synchronizationPanels.find(animationUUID) != m_synchronizationPanels.end())
				{
					return false;
				}
				if (ImGui::Selectable(animationPanel->GetAnimationName().c_str(), false))
				{
					if (m_editorAnimation.GetDuration() != animationPanel->GetAnimationDuration())
					{
						Log::Get().TerminalLog("%s", "Cannot synchonize animations with different durations.");
						Log::Get().ConsoleLog(LogLevel::Error, "%s", "Cannot synchronize animations with different durations.");
						return false;
					}
					m_synchronizationPanels.insert({ animationUUID, animationPanel.GetIndex()});
					animationPanel->AddSynchronizationPanel(GetRefIndex());
				}
				return false;
			}
		);
		ImGui::EndPopup();
	}
	if (m_synchronizationPanels.size() > 0)
	{
		ImGui::Indent();
		ImGui::Text("Synchronizations:");
		ImGui::Indent();
		animationPanelContainerType::Ref animationPanelToRemove;
		for (const auto& it : m_synchronizationPanels)
		{
			size_t panelIndex(it.second);
			animationPanelContainerType::Ref animationPanel(s_openedAnimationPanels.GetRefFromIndex(panelIndex));
			DASSERT_E(animationPanel.IsValid());
			ImGui::Selectable(animationPanel->GetAnimationName().c_str());
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::Selectable("Remove"))
				{
					animationPanelToRemove = animationPanel;
				}
				ImGui::EndPopup();
			}
		}
		if (animationPanelToRemove.IsValid())
		{
			m_synchronizationPanels.erase(animationPanelToRemove->GetAnimationUUID());
			animationPanelToRemove->RemoveSynchronizationPanel(GetRefIndex());
		}
		ImGui::Unindent();
		ImGui::Unindent();
	}
}

bool AnimationPanel::DrawComponentAndAttributes(DCore::ComponentIdType componentId, const std::vector<DCore::AttributeIdType>& attributes)
{
	if (!ImGui::CollapsingHeader(DCore::ComponentForms::Get()[componentId].Name.c_str()))
	{
		return false;
	}
	const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
	for (DCore::AttributeIdType attributeId : attributes)
	{
		const DCore::SerializedAttribute& attribute(componentForm.SerializedAttributes[attributeId]);
		const DCore::AttributeKeyframeType attributeKeyframeType(attribute.GetKeyframeType());
		const size_t numberOfAttributeComponents(attribute.GetNumberOfAttributeComponents());
		if (numberOfAttributeComponents == 1)
		{
			ImGui::Indent();
			DrawAttributeSelectable(attribute.GetAttributeName().GetName(), componentId, attributeId, 0, IsAttributeOfComponentSelected(componentId, attributeId, 0));
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::Selectable("Delete"))
				{
					m_editorAnimation.RemoveAnimation(componentId, attributeId);
					if (IsAttributeOfComponentSelected(componentId, attributeId, 0))
					{
						RemoveAttributeOfComponentFromSelected(componentId, attributeId, 0);
					}
					ImGui::EndPopup();
					return false;
				}
				if (ImGui::Selectable("Uniform Separate"))
				{
					const size_t numberOfKeyframes(m_editorAnimation.GetNumberOfKeyframes(componentId, attributeId, 0));
					const float animationDuration(m_editorAnimation.GetDuration());
					const float timeDistance(animationDuration / numberOfKeyframes);
					auto separateKeyframes
					(
						[&](auto keyframeContainer, auto setKeyframeFunction) -> void
						{
							for (size_t i(0); i < numberOfKeyframes; i++)
							{
								const size_t keyframeIndex(numberOfKeyframes - i - 1);
								const float keyframeTime(keyframeIndex * timeDistance);
								auto keyframe(keyframeContainer[keyframeIndex]);
								keyframe.SetTime(keyframeTime);
								std::invoke(setKeyframeFunction, keyframeIndex, keyframe);
							}
						}
					);
					switch (attribute.GetKeyframeType()) 
					{
					case DCore::AttributeKeyframeType::Integer:
						separateKeyframes
						(
							m_editorAnimation.GetIntegerKeyframes(componentId, attributeId, 0), 
							[&](size_t keyframeIndex, const IntegerKeyframe& keyframe) -> void
							{
								m_editorAnimation.SetIntegerKeyframeAtIndex(componentId, attributeId, 0, keyframeIndex, keyframe);
							}
						);
						break;
					case DCore::AttributeKeyframeType::Float:
						separateKeyframes
						(
							m_editorAnimation.GetFloatKeyframes(componentId, attributeId, 0), 
							[&](size_t keyframeIndex, const FloatKeyframe& keyframe) -> void
							{
								m_editorAnimation.SetFloatKeyframeAtIndex(componentId, attributeId, 0, keyframeIndex, keyframe);
							}
						);					
						break;
					default:
						DASSERT_E(false);
						break;
					}
				}
				if (attribute.GetKeyframeType() == DCore::AttributeKeyframeType::Integer)
				{
					static int from(0);
					static int to(0);
					if (ImGui::Selectable("Set Range", false, ImGuiSelectableFlags_DontClosePopups))
					{
						ImGui::OpenPopup("SetRangePopup");
					}
					if (ImGui::BeginPopup("SetRangePopup"))
					{
						ImGui::AlignTextToFramePadding();
						ImGui::Text("%s", "From");
						ImGui::SameLine();
						if (ImGui::DragInt("##From", &from))
						{
							from = std::min(from, to);
						}
						ImGui::SameLine();
						ImGui::AlignTextToFramePadding();
						ImGui::Text("%s", "To");
						ImGui::SameLine();
						if (ImGui::DragInt("##To", &to))
						{
							to = std::max(from, to);
						}
						if (ImGui::Button("Set"))
						{
							const float animationDuration(m_editorAnimation.GetDuration());
							const size_t nk(m_editorAnimation.GetNumberOfKeyframes(componentId, attributeId, 0));
							for (size_t i(1); i < nk; i++)
							{
								m_editorAnimation.RemoveKeyframe(componentId, attributeId, 0, 1);
							}
							const IntegerKeyframe firstKeyframe(0.0f, from);
							const size_t numberOfKeyframes(to - from + 1);
							const float timeDistance(animationDuration / numberOfKeyframes);
							m_editorAnimation.SetIntegerKeyframeAtIndex(componentId, attributeId, 0, 0, firstKeyframe);
							for (size_t i(0); i < numberOfKeyframes; i++)
							{
								const IntegerKeyframe keyframe(timeDistance * i, from + i);
								m_editorAnimation.TryAddIntegerKeyframe(componentId, attributeId, 0, keyframe);
							}
							m_selectedKeyframeInfos.clear();
							m_selectedKeyframes.clear();
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
				}
				ImGui::EndPopup();
			}
			ImGui::Unindent();
		}
		else
		{
			if (!ImGui::TreeNodeEx(attribute.GetAttributeName().GetName().c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow))
			{
				continue;
			}
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::Selectable("Delete"))
				{
					m_editorAnimation.RemoveAnimation(componentId, attributeId);
					for (size_t index(0); index < numberOfAttributeComponents; index++)
					{
						if (IsAttributeOfComponentSelected(componentId, attributeId, index))
						{
							RemoveAttributeOfComponentFromSelected(componentId, attributeId, index);
						}
					}
					ImGui::EndPopup();
					ImGui::TreePop();
					return false;
				}
				ImGui::EndPopup();
			}
			ImGui::Indent();
			for (size_t index(0); index < numberOfAttributeComponents; index++)
			{
				DrawAttributeSelectable(attribute.GetAttributeName().GetComponentAtIndex(index), componentId, attributeId, index, IsAttributeOfComponentSelected(componentId, attributeId, index));
			}
			ImGui::Unindent();
			ImGui::TreePop();
		}
	}
	return false;
}

void AnimationPanel::AddSelectedAttributeOfComponent(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, bool toRemoveOthers)
{
	if (toRemoveOthers)
	{
		m_numberOfSelectedAttributesComponents = 0;
		for (auto& attributeIds : m_selectedAttributes)
		{
			for (auto& sparseSet : attributeIds)
			{
				sparseSet.Clear();
			}
		}
		ClearKeyframesFromSelectionExcept(componentId, attributeId, attributeComponentId);
	}
	if (componentId + 1 > m_selectedAttributes.size())
	{
		m_selectedAttributes.resize(componentId + 1);
	}
	if (attributeId + 1 > m_selectedAttributes[componentId].size())
	{
		m_selectedAttributes[componentId].resize(attributeId + 1);
	}
	DASSERT_E(!m_selectedAttributes[componentId][attributeId].Exists(attributeComponentId));
	m_selectedAttributes[componentId][attributeId].Add(attributeComponentId);
	m_numberOfSelectedAttributesComponents++;
}

bool AnimationPanel::IsAttributeOfComponentSelected(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId) const
{
	if (componentId >= m_selectedAttributes.size() || attributeId >= m_selectedAttributes[componentId].size())
	{
		return false;
	}
	return m_selectedAttributes[componentId][attributeId].Exists(attributeComponentId);
}

void AnimationPanel::RemoveAttributeOfComponentFromSelected(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId)
{
	if (componentId >= m_selectedAttributes.size() || attributeId >= m_selectedAttributes[componentId].size())
	{
		return;
	}
	m_selectedAttributes[componentId][attributeId].Remove(attributeComponentId);
	m_numberOfSelectedAttributesComponents--;
	ClearKeyframesFromSelection(componentId, attributeId, attributeComponentId);
}
 
void AnimationPanel::DrawAttributeSelectable(const std::string attributeName, DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, bool isSelected)
{
	ImGuiIO& io(ImGui::GetIO());
	ImGui::Selectable(attributeName.c_str(), isSelected);
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
	{
		if (isSelected && io.KeyCtrl)
		{
			RemoveAttributeOfComponentFromSelected(componentId, attributeId, attributeComponentId);
		}
		else
		{
			AddSelectedAttributeOfComponent(componentId, attributeId, attributeComponentId, !io.KeyCtrl);
		}
	}
}

void AnimationPanel::DrawKeyframes(const DCore::DVec2& canvasCenterPos)
{
	static constexpr float keyframeRadius{6.0f};
	static constexpr float keyframeDiameter{2.0f * keyframeRadius};
	static constexpr float controlPointRadius{keyframeRadius / 3.0f};
	static constexpr float controlPointDiameter{2.0f * keyframeRadius};
	static const char* keyframeFormat{"EditorKeyframe%lu%lu%lu%lu"};
	static const char* leftControlPointFormat{"EditorKeyframeLeftControlPoint%lu%lu%lu%lu"};
	static const char* rightControlPointFormat{"EditorKeyframeRightControlPoint%lu%lu%lu%lu"};
	static constexpr ImVec2 movementDetailsPadding{keyframeDiameter, -2.5f * keyframeDiameter};
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	ImGuiIO& io(ImGui::GetIO());
	const float centerValueX(-m_scrolling.x/m_pixelsPerUnit);
	const float centerValueY(m_scrolling.y/m_pixelsPerUnit);
	m_editorAnimation.IterateOnComponentIdsAndAttributeIds
	(
		[&](DCore::ComponentIdType componentId, const std::vector<DCore::AttributeIdType>& attributes) -> bool
		{
			const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
			for (DCore::AttributeIdType attributeId : attributes)
			{
				const DCore::AttributeType attributeType(componentForm.SerializedAttributes[attributeId].GetAttributeType());
				const DCore::AttributeKeyframeType attributeKeyframeType(componentForm.SerializedAttributes[attributeId].GetKeyframeType());
				const size_t numberOfAttributeComponents(componentForm.SerializedAttributes[attributeId].GetNumberOfAttributeComponents());
				if (attributeKeyframeType == DCore::AttributeKeyframeType::Float)
				{
					for (size_t attributeComponentId(0); attributeComponentId < numberOfAttributeComponents; attributeComponentId++)
					{
						if (!IsAttributeOfComponentSelected(componentId, attributeId, attributeComponentId))
						{
							continue;
						}
						const std::vector<FloatKeyframe>& keyframes(m_editorAnimation.GetFloatKeyframes(componentId, attributeId, attributeComponentId));
						size_t keyframeIndex(0);
						for (const FloatKeyframe& keyframe : keyframes)
						{
							const DCore::DVec2 mainPoint(keyframe.GetMainPoint());
							const ImVec2 keyframePos{canvasCenterPos.x + (mainPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (mainPoint.y - centerValueY) * m_pixelsPerUnit};
							char keyframeId[128];
							std::sprintf(keyframeId, keyframeFormat, componentId, attributeId, attributeComponentId, keyframeIndex);
							drawList->AddNgonFilled(keyframePos, keyframeRadius, IM_COL32(255, 255, 255, 255), 4);
							ImGui::SetCursorScreenPos({keyframePos.x - keyframeRadius, keyframePos.y - keyframeRadius});
							ImGui::InvisibleButton(keyframeId, {keyframeDiameter, keyframeDiameter});
							if (ImGui::BeginPopupContextItem())
							{
								if (ImGui::Selectable("Reset"))
								{
									m_editorAnimation.SetFloatKeyframeAtIndex(componentId, attributeId, attributeComponentId, keyframeIndex, FloatKeyframe(keyframe.GetMainPoint().x, keyframe.GetMainPoint().y));
								}
								ImGui::EndPopup();
							}
							// Select keyframe
							if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex))
							{
								if (IsKeyframeSelected(componentId, attributeId, attributeComponentId, keyframeIndex))
								{
									if (io.KeyCtrl)
									{
										RemoveSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeIndex);
									}
									else
									{
										ClearKeyframesFromSelection();
										AddSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeIndex, true);
									}
								}
								else
								{
									AddSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeIndex, !io.KeyCtrl);
								}
							}
							// Move keyframe
							if (ImGui::IsItemActive() && IsKeyframeSelected(componentId, attributeId, attributeComponentId, keyframeIndex) && !io.KeyCtrl)
							{
								const DCore::DVec2 currentMainPoint(keyframe.GetMainPoint());
								const DCore::DVec2 newMainPoint{centerValueX + (io.MousePos.x - canvasCenterPos.x) / m_pixelsPerUnit, centerValueY - (io.MousePos.y - canvasCenterPos.y) / m_pixelsPerUnit};
								if (currentMainPoint != newMainPoint)
								{
									if (!IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex))
									{
										AddKeyrameBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex);
									}
									const DCore::DVec2 leftControlPointDiff(keyframe.GetLeftControlPoint() - mainPoint);							
									const DCore::DVec2 rightControlPointDiff(keyframe.GetRightControlPoint() - mainPoint);
									const DCore::DVec2 newLeftControlPoint{newMainPoint + leftControlPointDiff};
									const DCore::DVec2 newRightControlPoint{newMainPoint + rightControlPointDiff};
									FloatKeyframe newKeyframe(newMainPoint.x, newMainPoint.y);
									newKeyframe.SetLeftControlPoint(newLeftControlPoint.x, newLeftControlPoint.y);
									newKeyframe.SetRightControlPoint(newRightControlPoint.x, newRightControlPoint.y);
									m_editorAnimation.SetFloatKeyframeAtIndex(componentId, attributeId, attributeComponentId, keyframeIndex, newKeyframe);
								}
							}
							else if (IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex))
							{
								RemoveKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex);
							}
							// Draw control points
							if (IsKeyframeSelected(componentId, attributeId, attributeComponentId, keyframeIndex))
							{

								const DCore::DVec2 leftControlPoint(keyframe.GetLeftControlPoint());
								const DCore::DVec2 rightControlPoint(keyframe.GetRightControlPoint());
								for (uint8_t count(0); count < 2; count++)
								{
									const DCore::DVec2 controlPoint(count == 0 ? leftControlPoint : rightControlPoint);
									const ImVec2 controlPointPos{canvasCenterPos.x + (controlPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (controlPoint.y - centerValueY) * m_pixelsPerUnit};
									drawList->AddNgonFilled(controlPointPos, controlPointRadius, IM_COL32(255, 255, 255, 255), 4);
									drawList->AddLine(keyframePos, controlPointPos, IM_COL32(255, 255, 255, 75));
									char controlPointId[128];
									std::sprintf(controlPointId, count == 0 ? leftControlPointFormat : rightControlPointFormat, componentId, attributeId, attributeComponentId, keyframeIndex);
									ImGui::SetCursorScreenPos({controlPointPos.x - controlPointRadius, controlPointPos.y - controlPointRadius});
									ImGui::InvisibleButton(controlPointId, {controlPointDiameter, controlPointDiameter});
									if (ImGui::IsItemActive())
									{
										const DCore::DVec2 newControlPointValue{centerValueX + (io.MousePos.x - canvasCenterPos.x) / m_pixelsPerUnit, centerValueY - (io.MousePos.y - canvasCenterPos.y) / m_pixelsPerUnit};
										FloatKeyframe newKeyframe(mainPoint.x, mainPoint.y);
										if (count == 0)
										{
											newKeyframe.SetLeftControlPoint(newControlPointValue.x, newControlPointValue.y);
											newKeyframe.SetRightControlPoint(rightControlPoint.x, rightControlPoint.y);
										}
										else
										{
											newKeyframe.SetLeftControlPoint(leftControlPoint.x, leftControlPoint.y);
											newKeyframe.SetRightControlPoint(newControlPointValue.x, newControlPointValue.y);
										}
										m_editorAnimation.SetFloatKeyframeAtIndex(componentId, attributeId, attributeComponentId, keyframeIndex, newKeyframe);
									}
								}
							}
							keyframeIndex++;
							if (keyframeIndex >= keyframes.size())
							{
								// Draw line
								const float animationDurationTimePos(canvasCenterPos.x + (m_editorAnimation.GetDuration() - centerValueX) * m_pixelsPerUnit);
								drawList->AddLine(keyframePos, {animationDurationTimePos, keyframePos.y}, IM_COL32(0, 255, 0, 255));
							}
							else
							{
								// Draw bezier curve
								const DCore::DVec2 keyframeRightControlPoint(keyframe.GetRightControlPoint());
								const ImVec2 keyframeRightControlPointPos{canvasCenterPos.x + (keyframeRightControlPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (keyframeRightControlPoint.y - centerValueY) * m_pixelsPerUnit};
								const FloatKeyframe& nextKeyframe(keyframes[keyframeIndex]);
								const DCore::DVec2 nextKeyframeMainPoint(nextKeyframe.GetMainPoint());
								const DCore::DVec2 nextKeyframeLeftControlPoint(nextKeyframe.GetLeftControlPoint());
								const ImVec2 nextKeyframePos{canvasCenterPos.x + (nextKeyframeMainPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (nextKeyframeMainPoint.y - centerValueY) * m_pixelsPerUnit};
								const ImVec2 nextKeyframeLeftControlPointPos{canvasCenterPos.x + (nextKeyframeLeftControlPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (nextKeyframeLeftControlPoint.y - centerValueY) * m_pixelsPerUnit};
								drawList->AddBezierCubic(keyframePos, keyframeRightControlPointPos, nextKeyframeLeftControlPointPos, nextKeyframePos, IM_COL32(0, 255, 0, 255), 1.0f);
							}
							// Draw movement details
							if (IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex - 1))
							{
								const DCore::DVec2 mainPoint(keyframe.GetMainPoint());
								const ImVec2 keyframePos{canvasCenterPos.x + (mainPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (mainPoint.y - centerValueY) * m_pixelsPerUnit};
								ImGui::SetCursorScreenPos({keyframePos.x + movementDetailsPadding.x, keyframePos.y + movementDetailsPadding.y});
								ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "Time: %.3f", mainPoint.x);
								ImGui::SetCursorScreenPos({keyframePos.x + movementDetailsPadding.x, keyframePos.y + movementDetailsPadding.y + 15.0f});
								ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "Value: %.3f", mainPoint.y);							
							}
						}
					}
				}
				else 
				{
					for (size_t attributeComponentId(0); attributeComponentId < numberOfAttributeComponents; attributeComponentId++)
					{
						if (!IsAttributeOfComponentSelected(componentId, attributeId, attributeComponentId))
						{
							continue;
						}
						const std::vector<IntegerKeyframe>& keyframes(m_editorAnimation.GetIntegerKeyframes(componentId, attributeId, attributeComponentId));
						size_t keyframeIndex(0);
						for (const IntegerKeyframe& keyframe : keyframes)
						{
							const DCore::DVec2 mainPoint(keyframe.GetMainPoint());
							const ImVec2 keyframePos{canvasCenterPos.x + (mainPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (mainPoint.y - centerValueY) * m_pixelsPerUnit};
							char keyframeId[128];
							std::sprintf(keyframeId, keyframeFormat, componentId, attributeId, attributeComponentId, keyframeIndex);
							drawList->AddNgonFilled(keyframePos, keyframeRadius, IM_COL32(255, 255, 255, 255), 4);
							ImGui::SetCursorScreenPos({keyframePos.x - keyframeRadius, keyframePos.y - keyframeRadius});
							ImGui::InvisibleButton(keyframeId, {keyframeDiameter, keyframeDiameter});
							// Select keyframe
							if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex))
							{
								if (IsKeyframeSelected(componentId, attributeId, attributeComponentId, keyframeIndex))
								{
									if (io.KeyCtrl)
									{
										RemoveSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeIndex);
									}
									else
									{
										ClearKeyframesFromSelection();
										AddSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeIndex, true);
									}
								}
								else
								{
									AddSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeIndex, !io.KeyCtrl);
								}
							}
							// Move keyframe
							if (ImGui::IsItemActive() && IsKeyframeSelected(componentId, attributeId, attributeComponentId, keyframeIndex) && !io.KeyCtrl)
							{
								const DCore::DVec2 currentMainPoint(keyframe.GetMainPoint());
								const DCore::DVec2 newMainPoint{centerValueX + (io.MousePos.x - canvasCenterPos.x) / m_pixelsPerUnit, centerValueY - (io.MousePos.y - canvasCenterPos.y) / m_pixelsPerUnit};
								if (currentMainPoint != newMainPoint)
								{
									if (!IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex))
									{
										AddKeyrameBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex);
									}
									IntegerKeyframe newKeyframe(newMainPoint.x, newMainPoint.y);
									m_editorAnimation.SetIntegerKeyframeAtIndex(componentId, attributeId, attributeComponentId, keyframeIndex, newKeyframe);
									// Draw movement details
								}
							}
							else if (IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex))
							{
								RemoveKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex);
							}
							keyframeIndex++;
							// Draw movement details
							if (IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeIndex - 1))
							{
								const DCore::DVec2 mainPoint(keyframe.GetMainPoint());
								const ImVec2 keyframePos{canvasCenterPos.x + (mainPoint.x - centerValueX) * m_pixelsPerUnit, canvasCenterPos.y - (mainPoint.y - centerValueY) * m_pixelsPerUnit};
								ImGui::SetCursorScreenPos({keyframePos.x + movementDetailsPadding.x, keyframePos.y + movementDetailsPadding.y});
								ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "Time: %.3f", mainPoint.x);
								ImGui::SetCursorScreenPos({keyframePos.x + movementDetailsPadding.x, keyframePos.y + movementDetailsPadding.y + 15.0f});
								ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "Value: %d", static_cast<int>(mainPoint.y));							
							}
						}
					}
				}
			}
			return false;
		}
	);
}

void AnimationPanel::AddSelectedKeyframe(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t keyframeIndex, bool toRemoveOthersFromSelection)
{
	if (toRemoveOthersFromSelection)
	{
		ClearKeyframesFromSelection();
	}
	if (componentId + 1 > m_selectedKeyframes.size())
	{
		m_selectedKeyframes.resize(componentId + 1);
	}
	if (attributeId + 1 > m_selectedKeyframes[componentId].size())
	{
		m_selectedKeyframes[componentId].resize(attributeId + 1);
	}
	if (attributeComponentId + 1 > m_selectedKeyframes[componentId][attributeId].size())
	{
		m_selectedKeyframes[componentId][attributeId].resize(attributeComponentId + 1);
	}
	DASSERT_E(!m_selectedKeyframes[componentId][attributeId][attributeComponentId].Exists(keyframeIndex));
	m_selectedKeyframes[componentId][attributeId][attributeComponentId].Add(keyframeIndex);
	m_selectedKeyframeInfos.push_back(KeyframeInfo(componentId, attributeId, attributeComponentId, keyframeIndex));
}

void AnimationPanel::RemoveSelectedKeyframe(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t keyframeIndex)
{
	DASSERT_E(componentId < m_selectedKeyframes.size());
	DASSERT_E(attributeId < m_selectedKeyframes[componentId].size());
	DASSERT_E(attributeComponentId < m_selectedKeyframes[componentId][attributeId].size());
	DASSERT_E(m_selectedKeyframes[componentId][attributeId][attributeComponentId].Exists(keyframeIndex));
	m_selectedKeyframes[componentId][attributeId][attributeComponentId].Remove(keyframeIndex);
	for (auto it(m_selectedKeyframeInfos.begin()); it != m_selectedKeyframeInfos.end(); it++)
	{
		if (it->ComponentId == componentId &&
			it->AttributeId == attributeId &&
			it->AttributeComponentId == attributeComponentId &&
			it->KeyframeId == keyframeIndex)
		{
			m_selectedKeyframeInfos.erase(it);
			return;
		}
	}
}

bool AnimationPanel::IsKeyframeSelected(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t keyframeIndex) const
{
	if (componentId + 1 > m_selectedKeyframes.size() || 
		attributeId + 1 > m_selectedKeyframes[componentId].size() || 
		attributeComponentId + 1 > m_selectedKeyframes[componentId][attributeId].size() ||
		!m_selectedKeyframes[componentId][attributeId][attributeComponentId].Exists(keyframeIndex))
	{
		return false;
	}
	return true;
}

void AnimationPanel::ClearKeyframesFromSelection()
{
	for (auto& attributeIds : m_selectedKeyframes)
	{
		for (auto& attributeComponentIds : attributeIds)
		{
			for (auto& keyframeIds : attributeComponentIds)
			{
				keyframeIds.Clear();
			}
		}
	}
	m_selectedKeyframeInfos.clear();
}

void AnimationPanel::ClearKeyframesFromSelection(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId)
{
	if (componentId >= m_selectedKeyframes.size() ||
		attributeId >= m_selectedKeyframes[componentId].size() ||
		attributeComponentId >= m_selectedKeyframes[componentId][attributeId].size())
	{
		return;
	}
	m_selectedKeyframes[componentId][attributeId][attributeComponentId].Clear();
	std::vector<std::vector<KeyframeInfo>::iterator> infosToRemove;
	for (auto it(m_selectedKeyframeInfos.begin()); it != m_selectedKeyframeInfos.end(); it++)
	{
		if (it->ComponentId == componentId &&
			it->AttributeId == attributeId &&
			it->AttributeComponentId == attributeComponentId)
		{
			infosToRemove.push_back(it);
		}
	}
	for (auto& it : infosToRemove)
	{
		m_selectedKeyframeInfos.erase(it);
	}
}

void AnimationPanel::ClearKeyframesFromSelectionExcept(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId)
{
	std::vector<size_t> selectedKeyframesIndexes;
	const DCore::AttributeKeyframeType attributeKeyframeType(DCore::ComponentForms::Get()[componentId].SerializedAttributes[attributeId].GetKeyframeType());
	if (attributeKeyframeType == DCore::AttributeKeyframeType::Integer)
	{
		const std::vector<IntegerKeyframe>& keyframes(m_editorAnimation.GetIntegerKeyframes(componentId, attributeId, attributeComponentId));
		for (size_t index(0); index < keyframes.size(); index++)
		{
			if (IsKeyframeSelected(componentId, attributeId, attributeComponentId, index))
			{
				selectedKeyframesIndexes.push_back(index);
			}
		}
	}
	else
	{
		const std::vector<FloatKeyframe>& keyframes(m_editorAnimation.GetFloatKeyframes(componentId, attributeId, attributeComponentId));
		for (size_t index(0); index < keyframes.size(); index++)
		{
			if (IsKeyframeSelected(componentId, attributeId, attributeComponentId, index))
			{
				selectedKeyframesIndexes.push_back(index);
			}
		}
	}
	m_selectedKeyframes.clear();
	for (size_t selectedKeyframeIndex : selectedKeyframesIndexes)
	{
		AddSelectedKeyframe(componentId, attributeId, attributeComponentId, selectedKeyframeIndex, false);
	}
	std::vector<std::vector<KeyframeInfo>::iterator> infosToRemove;
	for (auto it(m_selectedKeyframeInfos.begin()); it != m_selectedKeyframeInfos.end(); it++)
	{
		if (it->ComponentId != componentId &&
			it->AttributeId != attributeId &&
			it->AttributeComponentId != attributeComponentId)
		{
			infosToRemove.push_back(it);
		}
	}
	for (auto& it : infosToRemove)
	{
		m_selectedKeyframeInfos.erase(it);
	}
}

void AnimationPanel::DeleteSelectedKeyframes()
{
	for (const KeyframeInfo& selectedKeyframeInfo : m_selectedKeyframeInfos)
	{
		m_selectedKeyframes[selectedKeyframeInfo.ComponentId][selectedKeyframeInfo.AttributeId][selectedKeyframeInfo.AttributeComponentId].Remove(selectedKeyframeInfo.KeyframeId);
		if (selectedKeyframeInfo.KeyframeId == 0)
		{
			static const char* simpleAttributeFormat{"Try to delete keyframe at index 0 (\"%s\" -> \"%s\"). Keyframes at this index cannot be deleted."};
			static const char* compositeAttributeFormat{"Try to delete keyframe at index 0 (\"%s\" -> \"%s\" -> \"%s\"). Keyframes at this index cannot be deleted."};
			const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[selectedKeyframeInfo.ComponentId]);
			const DCore::SerializedAttribute& attribute(componentForm.SerializedAttributes[selectedKeyframeInfo.AttributeId]);
			if (componentForm.SerializedAttributes[selectedKeyframeInfo.AttributeId].GetNumberOfAttributeComponents() == 1)
			{
				Log::Get().ConsoleLog(LogLevel::Error, simpleAttributeFormat, componentForm.Name.c_str(), attribute.GetAttributeName().GetName().c_str());
				continue;
			}
			Log::Get().ConsoleLog(LogLevel::Error, compositeAttributeFormat, componentForm.Name.c_str(), attribute.GetAttributeName().GetName().c_str(), attribute.GetAttributeName().GetComponentAtIndex(selectedKeyframeInfo.AttributeComponentId).c_str());
			continue;
		}
		m_editorAnimation.RemoveKeyframe(selectedKeyframeInfo.ComponentId, selectedKeyframeInfo.AttributeId, selectedKeyframeInfo.AttributeComponentId, selectedKeyframeInfo.KeyframeId);
		for (KeyframeInfo& ski : m_selectedKeyframeInfos)
		{
			if (&ski == &selectedKeyframeInfo)
			{
				continue;
			}
			if (ski.ComponentId == selectedKeyframeInfo.ComponentId &&
				ski.AttributeId == selectedKeyframeInfo.AttributeId &&
				ski.AttributeComponentId == selectedKeyframeInfo.AttributeComponentId &&
				ski.KeyframeId > selectedKeyframeInfo.KeyframeId)
			{
				ski.KeyframeId--;
			}
		}
	}
	m_selectedKeyframeInfos.clear();
}

void AnimationPanel::HandleKeyframeBoxSelection(const DCore::DVec2& canvasCenterPos)
{
	ImGuiIO& io(ImGui::GetIO());
	const DCore::DVec2 canvasCenterValue{-m_scrolling.x / m_pixelsPerUnit, m_scrolling.y / m_pixelsPerUnit};
	DCore::DVec2 boxSelectionMinPos;
	DCore::DVec2 boxSelectionMaxPos;
	if (m_boxSelectionInitialPos.x <= io.MousePos.x && m_boxSelectionInitialPos.y <= io.MousePos.y)
	{
		boxSelectionMinPos = m_boxSelectionInitialPos;
		boxSelectionMaxPos = {io.MousePos.x, io.MousePos.y};
	}
	else if (m_boxSelectionInitialPos.x >= io.MousePos.x && m_boxSelectionInitialPos.y >= io.MousePos.y)
	{
		boxSelectionMinPos = {io.MousePos.x, io.MousePos.y};
		boxSelectionMaxPos = m_boxSelectionInitialPos;
	}
	else if (m_boxSelectionInitialPos.x >= io.MousePos.x && m_boxSelectionInitialPos.y <= io.MousePos.y)
	{
		boxSelectionMinPos = {io.MousePos.x, m_boxSelectionInitialPos.y};
		boxSelectionMaxPos = {m_boxSelectionInitialPos.x, io.MousePos.y};
	}
	else if (m_boxSelectionInitialPos.x <= io.MousePos.x && m_boxSelectionInitialPos.y >= io.MousePos.y)
	{
		boxSelectionMinPos = {m_boxSelectionInitialPos.x, io.MousePos.y};
		boxSelectionMaxPos = {io.MousePos.x, m_boxSelectionInitialPos.y};
	}
	m_editorAnimation.IterateOnComponentIdsAndAttributeIds
	(
		[&](DCore::ComponentIdType componentId, const std::vector<DCore::AttributeIdType>& attributeIds) -> bool 
		{
			const DCore::ComponentForm& componentForm(DCore::ComponentForms::Get()[componentId]);
			for (DCore::AttributeIdType attributeId : attributeIds)
			{
				const DCore::AttributeKeyframeType attributeKeyframeType(componentForm.SerializedAttributes[attributeId].GetKeyframeType());
				DASSERT_E(attributeKeyframeType != DCore::AttributeKeyframeType::NotDefined);
				for (size_t attributeComponentId(0); attributeComponentId < componentForm.SerializedAttributes[attributeId].GetNumberOfAttributeComponents(); attributeComponentId++)
				{
					if (!IsAttributeOfComponentSelected(componentId, attributeId, attributeComponentId))
					{
						continue;
					}
					if (attributeKeyframeType == DCore::AttributeKeyframeType::Float)
					{ 
						const std::vector<FloatKeyframe>& keyframes(m_editorAnimation.GetFloatKeyframes(componentId, attributeId, attributeComponentId));
						for (size_t keyframeId(0); keyframeId < keyframes.size(); keyframeId++)
						{
							if (IsKeyframeSelected(componentId, attributeId, attributeComponentId, keyframeId))
							{
								continue;
							}
							// Check if keyframe is in box selection bounds
							const DCore::DVec2 mainPoint(keyframes[keyframeId].GetMainPoint());
							const DCore::DVec2 mainPointPos{canvasCenterPos.x + (mainPoint.x - canvasCenterValue.x) * m_pixelsPerUnit, canvasCenterPos.y - (mainPoint.y - canvasCenterValue.y) * m_pixelsPerUnit};
							if (mainPointPos.x >= boxSelectionMinPos.x &&
								mainPointPos.x <= boxSelectionMaxPos.x &&
								mainPointPos.y >= boxSelectionMinPos.y &&
								mainPointPos.y <= boxSelectionMaxPos.y)
							{
								AddSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeId, false);
							}
						}
					}
					else
					{
						const std::vector<IntegerKeyframe>& keyframes(m_editorAnimation.GetIntegerKeyframes(componentId, attributeId, attributeComponentId));
						for (size_t keyframeId(0); keyframeId < keyframes.size(); keyframeId++)
						{
							if (IsKeyframeSelected(componentId, attributeId, attributeComponentId, keyframeId))
							{
								continue;
							}
							// Check if keyframe is in box selection bounds
							const DCore::DVec2 mainPoint(keyframes[keyframeId].GetMainPoint());
							const DCore::DVec2 mainPointPos{canvasCenterPos.x + (mainPoint.x - canvasCenterValue.x) * m_pixelsPerUnit, canvasCenterPos.y - (mainPoint.y - canvasCenterValue.y) * m_pixelsPerUnit};
							if (mainPointPos.x >= boxSelectionMinPos.x &&
								mainPointPos.x <= boxSelectionMaxPos.x &&
								mainPointPos.y >= boxSelectionMinPos.y &&
								mainPointPos.y <= boxSelectionMaxPos.y)
							{
								AddSelectedKeyframe(componentId, attributeId, attributeComponentId, keyframeId, false);
							}
						}
					}
				}
			}
			return false;
		}
	);
}

bool AnimationPanel::IsKeyframeBeeingMoved(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t keyframeId) const
{
	if (componentId >= m_keyframesBeeingMoved.size() ||
		attributeId >= m_keyframesBeeingMoved[componentId].size() ||
		attributeComponentId >= m_keyframesBeeingMoved[componentId][attributeId].size() ||
		!m_keyframesBeeingMoved[componentId][attributeId][attributeComponentId].Exists(keyframeId))
	{
		return false;
	}
	return true;
}

void AnimationPanel::AddKeyrameBeeingMoved(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t keyframeId)
{
	DASSERT_E(!IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeId));
	if (componentId + 1 >= m_keyframesBeeingMoved.size())
	{
		m_keyframesBeeingMoved.resize(componentId + 1);
	}
	if (attributeId + 1 >= m_keyframesBeeingMoved[componentId].size())
	{
		m_keyframesBeeingMoved[componentId].resize(attributeId + 1);
	}
	if (attributeComponentId + 1 >= m_keyframesBeeingMoved[componentId][attributeId].size())
	{
		m_keyframesBeeingMoved[componentId][attributeId].resize(attributeComponentId + 1);
	}
	m_keyframesBeeingMoved[componentId][attributeId][attributeComponentId].Add(keyframeId);
}

void AnimationPanel::RemoveKeyframeBeeingMoved(DCore::ComponentIdType componentId, DCore::AttributeIdType attributeId, size_t attributeComponentId, size_t keyframeId)
{
	DASSERT_E(IsKeyframeBeeingMoved(componentId, attributeId, attributeComponentId, keyframeId));
	m_keyframesBeeingMoved[componentId][attributeId][attributeComponentId].Remove(keyframeId);
}

size_t AnimationPanel::GetRefIndex() const
{
	size_t index(0);
	s_openedAnimationPanels.Iterate
	(
		[&](animationPanelContainerType::Ref animationPanel) -> bool
		{
			if (animationPanel->GetAnimationUUID() == GetAnimationUUID())
			{
				index = animationPanel.GetIndex();
				return true;
			}
			return false;
		}
	);
	return index;
}

void AnimationPanel::SetSynchronizedCursorTime(float time)
{
	for (const auto& it : m_synchronizationPanels)
	{
		animationPanelContainerType::Ref animationPanel(s_openedAnimationPanels.GetRefFromIndex(it.second));
		DASSERT_E(animationPanel.IsValid());
		animationPanel->SetCursorTime(time);
	}
}

}
