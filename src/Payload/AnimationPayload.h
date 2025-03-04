#pragma once

#include "DommusCore.h"



using AnimationPayload = struct AnimationPayload
{
	using uuidType = DCore::UUIDType;
	
	AnimationPayload(const uuidType& uuid)
		:
		UUID(uuid)
	{}

	uuidType UUID;
};
