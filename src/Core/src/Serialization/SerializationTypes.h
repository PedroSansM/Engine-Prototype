#pragma once

#include "FixedString.h"
#include "AssetManagerTypes.h"
#include "ECSTypes.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#include <cstddef>
#include <cstdint>



struct b2BodyId;
struct b2ShapeId;

namespace DCore {

#define STRING_SIZE 256

class EntityRef;
class AnimationStateMachineRef;
class SoundEventInstance;
class SpriteMaterialRef;

template <size_t Size>
struct DTaggedList
{
	size_t NumberOfTags;
	size_t Selected;
	const char* Tags[Size];
};

enum class DBodyType : size_t
{
	Static = 0,
	Kinematic,
	Dynamic
};

namespace Physics
{
	enum class PhysicsLayer : uint64_t;
}

using AttributeIdType = size_t;
using DInt = int;
using DFloat = float;
using DDouble = double;
using DUInt = uint64_t;
using DLogic = bool;
using DSize = size_t;
using DString = FixedString<STRING_SIZE>;
using DVec2 = glm::vec2;
using DVec3 = glm::vec3;
using DVec4 = glm::vec4;
using DMat4 = glm::mat4;
using DEntity = EntityRef;
using DAnimationStateMachine = AnimationStateMachineRef;
using DBodyId = b2BodyId;
using DShapeId = b2ShapeId;
using DPhysicsLayer = Physics::PhysicsLayer;
using DSoundEventInstance = SoundEventInstance;
using DSpriteMaterial = SpriteMaterialRef;
//

} 
