#include "Panels.h"
#include "SceneHierarchyPanel.h"
#include "UtilityPanel.h"
#include "InspectorPanel.h"
#include "ResourcesPanel.h"
#include "ConsolePanel.h"
#include "EditorGameViewPanels.h"
#include "GameViewPanel.h"
#include "ConfigurationPanel.h"
#include "GameStatePanel.h"
#include "AnimationPanel.h"
#include "AnimationStateMachinePanel.h"
#include "PhysicsMaterialPanel.h"
#include "SpriteMaterialPanel.h"
#include "TexturePanel.h"
#include "SpriteSheetGenPanel.h"



namespace DEditor
{

void Panels::RenderPanels()
{
	SceneHierarchyPanel::Get().Render();
	UtilityPanel::Get().Render();
	InspectorPanel::Get().Render();
	ResourcesPanel::Get().Render();
	ConsolePanel::Get().Render();
	EditorGameViewPanels::Get().Render();
	GameViewPanel::Get().Render();
	ConfigurationPanel::Get().Render();
	GameStatePanel::Get().Render();
	AnimationPanel::RenderAnimationPanels();
	AnimationStateMachinePanel::RenderPanels();
	PhysicsMaterialPanel::RenderPanels();
	SpriteMaterialPanel::RenderPanels();
	TexturePanel::RenderTexturePanels();
	SpriteSheetGenPanel::RenderPanels();
}	

}
