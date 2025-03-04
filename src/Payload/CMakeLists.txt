target_sources(${MAIN_TARGET}
	PRIVATE
	AnimationPayload.h
	AnimationStateMachinePayload.h
	PhysicsMaterialPayload.h
)

target_include_directories(${MAIN_TARGET}
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
