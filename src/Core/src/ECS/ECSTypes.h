#pragma once

#include "ReciclingVector.h"
#include "EntityInfo.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>



namespace DCore
{

struct EntityInfo;

using ComponentIdType = size_t;
using EntityIdType = uint32_t;
using EntityVersionType = uint32_t;

using Entity = ReciclingVector<EntityInfo, EntityIdType, EntityVersionType>::ConstRef;

}
