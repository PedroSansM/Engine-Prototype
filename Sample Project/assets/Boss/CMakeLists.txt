target_sources(${MAIN_TARGET}
	PUBLIC
	FlowerComponent.cpp
	FlowerComponent.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(Attacks/Acorn)
add_subdirectory(Attacks/Boomerang)
add_subdirectory(Attacks/Seeds/Chomper)
add_subdirectory(Attacks/Burst)
add_subdirectory(Attacks/Seeds)
add_subdirectory(Attacks/Seeds/Venus)
