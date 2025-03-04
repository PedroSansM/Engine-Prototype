target_sources(${MAIN_TARGET}
   PRIVATE
   ImGuizmo.cpp
   ImGuizmo.h
)

target_include_directories(${MAIN_TARGET}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
