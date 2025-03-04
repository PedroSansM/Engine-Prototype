#include "ReturnError.h"



namespace DCore
{

ReturnError::ReturnError()
	:
	Ok(true)
{}

ReturnError::ReturnError(bool ok, DString&& message)
	:
	Ok(ok),
	Message(message)
{}

ReturnError::ReturnError(ReturnError&& other)
	:
	Ok(other.Ok),
	Message(std::move(other.Message))
{}

}
