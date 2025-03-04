#pragma once

#include "EntityRef.h"
#include "ScriptComponent.h"
#include "ReciclingVector.h"
#include "SparseSet.h"



namespace DCore
{

class Runtime;

struct UserData
{
	using scriptComponentContainerType = ReciclingVector<ComponentRef<ScriptComponent>>;

	EntityRef Entity;
	size_t Index;
	DCore::Runtime* Runtime;
	scriptComponentContainerType CollisionBeginScriptComponents;
	scriptComponentContainerType CollisionEndScriptComponents;
	scriptComponentContainerType OverlapBeginScriptComponents;
	scriptComponentContainerType OverlapEndScriptComponents;
};

}
