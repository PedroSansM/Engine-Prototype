#pragma once

#include "DommusCore.h"



namespace DEditor::Draw
{

using dVec2 = DCore::DVec2;

void DrawPlaySymbol(const dVec2& sizes, const dVec2& firstVertexScreenPos);
void DrawStopSymbol(const dVec2& barSizes, float barsDistance, const dVec2& firstVertexScreenPos);

}
