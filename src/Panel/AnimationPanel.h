#pragma once

#include "Panel.h"
#include "EditorAnimation.h"

#include "DommusCore.h"

#include <vector>
#include <unordered_map>



namespace DEditor
{

using KeyframeInfo = struct KeyframeInfo
{
	KeyframeInfo(DCore::ComponentIdType, DCore::AttributeIdType, size_t, size_t);
	KeyframeInfo(const KeyframeInfo&);
	~KeyframeInfo() = default;

	DCore::ComponentIdType ComponentId;
	DCore::AttributeIdType AttributeId;
	size_t AttributeComponentId;
	size_t KeyframeId;
	
	KeyframeInfo& operator=(const KeyframeInfo& other)
	{
		ComponentId = other.ComponentId;
		AttributeId = other.AttributeId;
		AttributeComponentId = other.AttributeComponentId;
		KeyframeId = other.KeyframeId;
		return *this;
	}
};

class AnimationPanel : public Panel
{
public:
	using animationPanelContainerType = DCore::ReciclingVector<AnimationPanel>;
	using coreAnimationRefType = DCore::AnimationRef;
	using entityRefType = DCore::EntityRef;
public:
	AnimationPanel(const char* panelName, Animation&& editorAnimation);
	AnimationPanel(AnimationPanel&&) noexcept;
	~AnimationPanel() = default;
public:
	static void OpenAnimationPanel(const char* animationName, Animation&& editorAnimation);
	static bool IsPanelWithAnimationUUIDOpened(const DCore::UUIDType&);
	static void RenderAnimationPanels();
public:
	/// Returns if the panel have to be closed.
	bool Render();
	void AddSynchronizationPanel(size_t animationPanelIndex);
	void RemoveSynchronizationPanel(size_t animationPanelIndex);
	void RemoveFromAllSynchronizationPanels();
	void SetCursorTime(float time);
public:
	const DCore::UUIDType& GetAnimationUUID() const
	{
		return m_editorAnimation.GetUUID();
	}
	
	const std::string& GetAnimationName() const
	{
		return m_editorAnimation.GetName();
	}

	const float GetAnimationDuration() const
	{
		return m_editorAnimation.GetDuration();
	}
private:
	static animationPanelContainerType s_openedAnimationPanels;
private:
	std::string m_panelName;
	Animation m_editorAnimation;
	coreAnimationRefType m_coreAnimation;
	bool m_isOpened;
 	float m_stepSize; // In units
	float m_pixelsPerUnit;
	DCore::DVec2 m_scrolling;
	int m_stepSizeOrder;
	std::vector<std::vector<DCore::SparseSet<size_t>>> m_selectedAttributes; // Component Id -> Attribute Id -> Attribute Component Id
	size_t m_numberOfSelectedAttributesComponents;
	float m_cursorTime;
	bool m_isBoxSelectionActive;
	DCore::DVec2 m_boxSelectionInitialPos;
	std::vector<std::vector<std::vector<DCore::SparseSet<size_t>>>> m_selectedKeyframes; // ComponentId -> Attribute Id -> Attribute ComponentId -> Keyframe Id
	std::vector<KeyframeInfo> m_selectedKeyframeInfos;
	std::vector<std::vector<std::vector<DCore::SparseSet<size_t>>>> m_keyframesBeeingMoved; // ComponentId -> Attribute Id -> Attribute ComponentId -> Keyframe Id
	entityRefType m_attachedEntity;
	bool m_isPlaying;
	std::unordered_map<DCore::UUIDType, size_t> m_synchronizationPanels;
private:
	void AddIntegerKeyframe(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, const IntegerKeyframe&);
	void AddFloatKeyframe(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, const FloatKeyframe&);
	void DrawCanvas();
	void DrawConfig();
	void DrawMetachannels();
	void DrawPlay();
	void DrawSynchronization();
	bool DrawComponentAndAttributes(DCore::ComponentIdType, const std::vector<DCore::AttributeIdType>&);
	void AddSelectedAttributeOfComponent(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, bool toRemoveOthers);
	bool IsAttributeOfComponentSelected(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId) const;
	void RemoveAttributeOfComponentFromSelected(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId);
	void DrawAttributeSelectable(const std::string attributeName, DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, bool isSelected);
	void DrawKeyframes(const DCore::DVec2& canvasCenterPos);
	void AddSelectedKeyframe(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, size_t keyframeIndex, bool toRemoveOthersFromSelection);
	void RemoveSelectedKeyframe(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, size_t keyframeIndex);
	bool IsKeyframeSelected(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, size_t keyframeIndex) const;
	void ClearKeyframesFromSelection();
	void ClearKeyframesFromSelection(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId);
	void ClearKeyframesFromSelectionExcept(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId);
	void DeleteSelectedKeyframes();
	void HandleKeyframeBoxSelection(const DCore::DVec2& canvasCenterPos);
	bool IsKeyframeBeeingMoved(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, size_t keyframeId) const;
	void AddKeyrameBeeingMoved(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, size_t keyframeId);
	void RemoveKeyframeBeeingMoved(DCore::ComponentIdType, DCore::AttributeIdType, size_t attributeComponentId, size_t keyframeId);
	size_t GetRefIndex() const;
	void SetSynchronizedCursorTime(float time);
};

}
