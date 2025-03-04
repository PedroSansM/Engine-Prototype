target_sources(DommusCore
	PRIVATE
	ComponentForm.cpp
	ComponentForm.h
	SerializationTypes.h
)

target_include_directories(DommusCore
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)
