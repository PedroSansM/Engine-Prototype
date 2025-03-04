#pragma once

#include "EditorGameViewPanel.h"

#include <cstddef>



namespace DEditor
{

class EditorGameViewPanels
{
public:
	static constexpr size_t numberOfPanels{3};		
public:
	~EditorGameViewPanels() = default;
public:
	static EditorGameViewPanels& Get()
	{
		static EditorGameViewPanels editorGameViewPanels;
		return editorGameViewPanels;
	}
public:
	void Open(size_t panelIndex);
	void Render();
	size_t NumberOfOpenedPanels();
	void OnGizmoActivation(size_t panelId);
private:
	EditorGameViewPanels(); 
private:
	EditorGameViewPanel m_viewports[numberOfPanels];
	size_t m_renderingEditorGameViewPanelIndex;
};

}
