#include "ScriptComponent.h"
#include "Runtime.h"
#include "TransformComponent.h"



namespace DCore
{

bool ScriptComponent::CastBox(float boxRotation, const DVec2& boxSizes, const DVec2& origin, const DVec2& direction, float maxDistance, uint64_t onlyColliderWithLayers, rayCastResultType* out)
{
	return m_runtime->CastBox(boxRotation, boxSizes, origin, direction, maxDistance, onlyColliderWithLayers, out);
}	

bool ScriptComponent::OverlapBox(float boxRotation, const DVec2& boxSizes, const DVec2& origin, uint64_t selfPhysicsLayer, uint64_t onlyCollideWithLayers, overlapResultType* result, size_t entitiesSize)
{
	return m_runtime->OverlapBox(boxRotation, boxSizes, origin, selfPhysicsLayer, onlyCollideWithLayers, result, entitiesSize);
}

void ScriptComponent::DrawDebugBox(const DVec2& translation, float rotation, const DVec2& sizes, const DVec4& color)
{
	if (m_runtime == nullptr)
	{
		return;
	}
	const TransformComponent transform({{translation.x, translation.y, 0.0f}, rotation, {1.0f, 1.0f}});
	m_runtime->AddDrawDebugBoxCommand({transform.GetModelMatrix(), sizes, color});
}

}
