#include "EditorGameViewPanels.h"

#include "DommusCore.h"



namespace DEditor
{

EditorGameViewPanels::EditorGameViewPanels()
	:
	m_viewports
	{
		EditorGameViewPanel("Editor Viewport 0", 0, true),
		EditorGameViewPanel("Editor Viewport 1", 1), 
		EditorGameViewPanel("Editor Viewport 2", 2)
	},
	m_renderingEditorGameViewPanelIndex(0)
{}

void EditorGameViewPanels::Open(size_t panelIndex)
{
	DASSERT_E(panelIndex < numberOfPanels);
	m_viewports[panelIndex].Open();
}

void EditorGameViewPanels::Render()
{
	for (size_t i(0); i < numberOfPanels; i++)
	{
		m_viewports[i].Update();
		if (m_viewports[i].IsOpened() && m_viewports[i].IsRenderingDone())
		{
			m_viewports[i].Render();
		}
	}
}

size_t EditorGameViewPanels::NumberOfOpenedPanels()
{
	size_t toReturn(0);
	for (size_t i(0); i < numberOfPanels; i++)
	{
		if (m_viewports[i].IsOpened())
		{
			toReturn++;
		}
	}
	return toReturn;
}

void EditorGameViewPanels::OnGizmoActivation(size_t panelId)
{
	for (size_t i(0); i < numberOfPanels; i++)
	{
		if (i == panelId)
		{
			continue;
		}
		m_viewports[i].DisableGizmo();
	}
}

}
