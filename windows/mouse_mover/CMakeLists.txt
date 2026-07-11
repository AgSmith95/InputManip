cmake_minimum_required(VERSION 3.25)

set(MOUSE_MOVER_EXE MouseMover)

set(
    SOURCE_FILES_MMOVER
    mmover.cpp
)
add_executable(
    ${MOUSE_MOVER_EXE}
    ${SOURCE_FILES_MMOVER}
)
set_target_properties(${MOUSE_MOVER_EXE} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(
    ${MOUSE_MOVER_EXE}
    ws2_32
)