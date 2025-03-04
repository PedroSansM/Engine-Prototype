#include "DCoreMath.h"

#include "glm/ext/scalar_constants.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/trigonometric.hpp"

#include <cfloat>



namespace DCore::Math
{

void Decompose(const DMat4& model, DVec2& outTranslation, DFloat& outRotation, DVec2& outScale)
{
	constexpr float smallFloat(0.001f);
	outTranslation = {model[3][0], model[3][1]};
	const bool modelZeroZeroIsSmall(glm::abs(model[0][0]) < smallFloat);
	const float rotationDen(modelZeroZeroIsSmall ? 0.0f : model[0][0]);
	outRotation = glm::atan(model[0][1] / rotationDen);
	if (!modelZeroZeroIsSmall)
	{
		const float den(glm::cos(outRotation));
		outScale.x = model[0][0] / den;
		outScale.y = model[1][1] / den;
	}
	else
	{
		const float den(glm::sin(outRotation));
		outScale.x = model[0][1] / den;
		outScale.y = -model[1][0] / den;
	}
	outRotation = glm::degrees(outRotation);
}

void GetTranslation(const DMat4 &model, DVec3 &outTranslation)
{
	outTranslation = {model[3][0], model[3][1], model[3][2]};
}

float Lerp(float from, float to, float ratio)
{
	if (ratio >= 1.0f)
	{
		return to;
	}
	if (ratio <= 0.0f)
	{
		return from;
	}
	return from + ratio * (to - from);
}

DCore::DVec3 Cross(const DCore::DVec3& a, const DCore::DVec3& b)
{
	return {a.y*b.z - a.z*b.y , a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

bool IsPointInsideTriangle(const DCore::DVec2& p, const std::array<DCore::DVec2, 3>& trianglePoints)
{
	const DCore::DVec2& a(trianglePoints[0]);
	const DCore::DVec2& b(trianglePoints[1]);
	const DCore::DVec2& c(trianglePoints[2]);
	float w1Numerator(a.x*(c.y-a.y)+(p.y-a.y)*(c.x-a.x)-p.x*(c.y-a.y));
	float w1Denominator((b.y-a.y)*(c.x-a.x)-(b.x-a.x)*(c.y-a.y));
	float w1(w1Numerator/w1Denominator);
	float w2Numerator(p.y-a.y-w1*(b.y-a.y));
	float w2Denominator(c.y-a.y);
	float w2(w2Numerator/w2Denominator);
	return w1 > 0.0f && w2 > 0.0f && w1 + w2 < 1.0f;
}

}
