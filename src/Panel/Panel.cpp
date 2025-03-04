#include "Panel.h"

#include "imgui.h"



namespace DEditor
{

DCore::DVec2 Panel::GetMouseLocalPosition()
{
	ImGuiIO& io(ImGui::GetIO());
	ImVec2 cursorPos(ImGui::GetCursorScreenPos());
	return {io.MousePos.x - cursorPos.x, io.MousePos.y - cursorPos.y};
}

}
