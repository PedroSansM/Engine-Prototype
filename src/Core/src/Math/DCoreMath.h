#pragma once

#include "SerializationTypes.h"

#include <array>



namespace DCore::Math
{
	
void Decompose(const DMat4& model, DVec2& outTranslation, DFloat& outRotation, DVec2& outScale);
void GetTranslation(const DMat4& model, DVec3& outTranslation);
float Lerp(float from, float to, float ratio);
DCore::DVec3 Cross(const DCore::DVec3& a, const DCore::DVec3& b);
bool IsPointInsideTriangle(const DCore::DVec2& p, const std::array<DCore::DVec2, 3>& trianglePoints);

}
