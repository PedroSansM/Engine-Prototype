#pragma once

#include "DommusCore.h"



namespace DEditor
{

class Panel
{
public:
	Panel() = default;
	virtual ~Panel() = default;
protected:
	DCore::DVec2 GetMouseLocalPosition();
};

}
