target_sources(${MAIN_TARGET}
	PUBLIC
	SeedComponent.cpp
	SeedComponent.h
	SeedEnemies.cpp
	SeedEnemies.h
	VinesComponent.cpp
	VinesComponent.h
	SeedEnemyAnimationComponent.cpp
	SeedEnemyAnimationComponent.h
	SeedEnemiesLauncherComponent.cpp
	SeedEnemiesLauncherComponent.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
