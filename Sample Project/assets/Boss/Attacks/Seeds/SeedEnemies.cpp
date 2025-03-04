#include "SeedEnemies.h"
#include "ChomperComponent.h"
#include "VenusComponent.h"



namespace Game
{

const DCore::ComponentIdType SeedEnemies::EnemiesIds[3]{ 
	DCore::ComponentId::GetId<ChomperComponent>(), 
	DCore::ComponentId::GetId<VenusComponent>(),
	0};

}