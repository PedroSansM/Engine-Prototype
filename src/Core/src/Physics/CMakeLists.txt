target_sources(DommusCore
	PRIVATE
	PhysicsAPI.cpp
	PhysicsAPI.h
	PhysicsMaterial.cpp
	PhysicsMaterial.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
