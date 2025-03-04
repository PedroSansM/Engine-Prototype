#pragma once

#include "DommusCore.h"



namespace DEditor
{

using AnimationStateMachinePayload = struct AnimationStateMachinePayload
{
	using uuidType = DCore::UUIDType;
	using stringType = DCore::DString;
	
	uuidType UUID;
	stringType Name;
};

}

