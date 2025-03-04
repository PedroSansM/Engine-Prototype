#pragma once

#include "SerializationTypes.h"
#include "Array.h"
#include "DCoreAssert.h"
#include "EntityRef.h"

#include "box2d/box2d.h"
#include "box2d/types.h"

#include <string>
#include <type_traits>
#include <tuple>


#define UNDEFINED_PHYSICS_LAYER_MASK B2_DEFAULT_CATEGORY_BITS
#define COLLIDE_WITH_ALL_MASK B2_DEFAULT_MASK_BITS

namespace DCore::Physics
{

b2BodyType CoreBodyTypeToBox2dBodyType(DBodyType);

constexpr size_t numberOfPhysicsLayers{33};

enum class PhysicsLayer : uint64_t
{
	Unspecified = 0,
	Layer1 = static_cast<uint64_t>(1) << 0,
	Layer2 = static_cast<uint64_t>(1) << 1,
	Layer3 = static_cast<uint64_t>(1) << 2,
	Layer4 = static_cast<uint64_t>(1) << 3,
	Layer5 = static_cast<uint64_t>(1) << 4,
	Layer6 = static_cast<uint64_t>(1) << 5,
	Layer7 = static_cast<uint64_t>(1) << 6,
	Layer8 = static_cast<uint64_t>(1) << 7,
	Layer9 = static_cast<uint64_t>(1) << 8,
	Layer10 = static_cast<uint64_t>(1) << 9,
	Layer11 = static_cast<uint64_t>(1) << 10,
	Layer12 = static_cast<uint64_t>(1) << 11,
	Layer13 = static_cast<uint64_t>(1) << 12,
	Layer14 = static_cast<uint64_t>(1) << 13,
	Layer15 = static_cast<uint64_t>(1) << 14,
	Layer16 = static_cast<uint64_t>(1) << 15,
	Layer17 = static_cast<uint64_t>(1) << 16,
	Layer18 = static_cast<uint64_t>(1) << 17,
	Layer19 = static_cast<uint64_t>(1) << 18,
	Layer20 = static_cast<uint64_t>(1) << 19,
	Layer21 = static_cast<uint64_t>(1) << 20,
	Layer22 = static_cast<uint64_t>(1) << 21,
	Layer23 = static_cast<uint64_t>(1) << 22,
	Layer24 = static_cast<uint64_t>(1) << 23,
	Layer25 = static_cast<uint64_t>(1) << 24,
	Layer26 = static_cast<uint64_t>(1) << 25,
	Layer27 = static_cast<uint64_t>(1) << 26,
	Layer28 = static_cast<uint64_t>(1) << 27,
	Layer29 = static_cast<uint64_t>(1) << 28,
	Layer30 = static_cast<uint64_t>(1) << 29,
	Layer31 = static_cast<uint64_t>(1) << 30,
	Layer32 = static_cast<uint64_t>(1) << 31,
};

struct RayCastResult
{
	EntityRef Entity;
	DVec2 Point;
	DVec2 Normal;
	// Internal.
	bool Hit;
};

struct OverlapResult
{
	OverlapResult()
		:
		Entities(nullptr)
	{}

	EntityRef* Entities;
};

// Internal
inline uint64_t Internal_GetLayersMask(uint64_t& returnValue)
{
	return returnValue;
}
//

// Public
template <class PhysicsLayerT, class ...PhysicsLayersT>
uint64_t GetLayersMask(PhysicsLayerT physicsLayer, PhysicsLayersT ...physicsLayers)
{
	static_assert(std::is_same_v<PhysicsLayerT, PhysicsLayer>);
	uint64_t returnValue(0);
	returnValue = static_cast<uint64_t>(physicsLayer);
	return Internal_GetLayersMask(returnValue, physicsLayers...);
}

inline uint64_t GetLayersMask(PhysicsLayer physicsLayers)
{
	return static_cast<uint64_t>(physicsLayers);
}
//

// Internal
template <class PhysicsLayerT, class ...PhysicsLayersT>
uint64_t Internal_GetLayersMask(uint64_t& returnValue, PhysicsLayerT physicsLayer, PhysicsLayersT ...physicsLayers)
{
	static_assert(std::is_same_v<PhysicsLayerT, PhysicsLayer>);
	returnValue |= static_cast<uint64_t>(physicsLayer);
	return Internal_GetLayersMask(returnValue, physicsLayers...);
}
//

}
