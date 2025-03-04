#pragma once

#include "SerializationTypes.h"
#include "Array.h"



namespace DCore
{

struct DrawDebugBoxCommand
{
	DMat4 Model;
	DVec2 Sizes;
	DVec4 Color;
};

}
