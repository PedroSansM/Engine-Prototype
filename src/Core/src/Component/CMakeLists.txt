target_sources(DommusCore
	PRIVATE
	AttributeRef.h
	Component.h
	ComponentRef.h
	ComponentRefSpecialization.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
