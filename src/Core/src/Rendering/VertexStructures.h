#pragma once

#include "glm/glm.hpp"

#include <cstdint>



namespace DCore
{

#pragma pack(push, 1)
using UnlitTexturedVertex = struct UnlitTexturedVertex
{
	uint32_t DrawOrder;
	glm::mat4 MVP;
	glm::vec3 VertexPos;
	glm::vec4 DiffuseColor;
	glm::vec4 TintColor;
	uint32_t ToUseDiffuseTex;
	uint32_t DiffuseTexId;
	glm::vec2 UV;
	uint32_t EntityId;
	uint32_t EntityVersion;
	uint32_t SceneId;
	uint32_t SceneVersion;
};
#pragma pack(pop)

template <class VertexType>
struct VertexComprator
{
	bool operator()(const VertexType& a, const VertexType& b) const
	{
		return a.DrawOrder < b.DrawOrder;
	}
};

#pragma pack(push, 1)
using DebugRectVertex = struct DebugRectVertex
{
	glm::mat4 MVP;
	glm::vec2 Offset;
	glm::vec2 RectSizes;
	glm::vec4 Color;
};
#pragma pack(pop)

};
