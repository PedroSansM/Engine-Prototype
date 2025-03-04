#include "Draw.h"

#include "imgui.h"



namespace DEditor::Draw
{

void DrawPlaySymbol(const dVec2& sizes, const dVec2& firstVertexScreenPos)
{
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	drawList->AddTriangleFilled({firstVertexScreenPos.x, firstVertexScreenPos.y}, {firstVertexScreenPos.x + sizes.x/2.0f, firstVertexScreenPos.y + sizes.y/2.0f}, {firstVertexScreenPos.x, firstVertexScreenPos.y + sizes.y}, IM_COL32(255, 255, 255, 255));
}

void DrawStopSymbol(const dVec2& barSizes, float barsDistance, const dVec2& firstVertexScreenPos)
{
	ImDrawList* drawList(ImGui::GetWindowDrawList());
	ImVec2 cursorPos{firstVertexScreenPos.x, firstVertexScreenPos.y};
	drawList->AddRectFilled(cursorPos, {cursorPos.x + barSizes.x, cursorPos.y + barSizes.y}, IM_COL32(255, 255, 255, 255));
	cursorPos = { cursorPos.x + barSizes.x + barsDistance, cursorPos.y };
	drawList->AddRectFilled(cursorPos, {cursorPos.x + barSizes.x, cursorPos.y + barSizes.y}, IM_COL32(255, 255, 255, 255));
}

}
