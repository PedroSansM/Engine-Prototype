target_sources(${MAIN_TARGET}
	PUBLIC
	PlayerAnimationComponent.cpp
	PlayerAnimationComponent.h
	PlayerComponent.cpp
	PlayerComponent.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
