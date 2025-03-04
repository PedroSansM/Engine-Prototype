target_sources(${MAIN_TARGET}
	PUBLIC
	PlayerHealthComponent.cpp
	PlayerHealthComponent.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
