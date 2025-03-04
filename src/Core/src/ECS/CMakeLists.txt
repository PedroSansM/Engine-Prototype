target_sources(DommusCore
	PRIVATE
	ComponentId.cpp
	ComponentId.h
	ECSTypes.h
	ComponentPool.h
	ECSUtils.h
	EntityInfo.h
	Registry.h
	Archetype.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
