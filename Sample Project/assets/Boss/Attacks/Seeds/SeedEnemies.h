#pragma once

#include "DommusCore.h"



namespace Game
{

typedef
struct SeedEnemies
{
	static constexpr size_t NumberOfSeedEnemies{3};
	static const DCore::ComponentIdType EnemiesIds[NumberOfSeedEnemies];
} SeedEnemies;

}
	