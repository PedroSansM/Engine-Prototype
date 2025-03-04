#pragma once

#include "SerializationTypes.h"



namespace DCore
{

using ReturnError = struct ReturnError
{
	ReturnError();
	ReturnError(bool, DString&&);
	ReturnError(ReturnError&&);

	bool Ok;
	DString Message;
};

}
