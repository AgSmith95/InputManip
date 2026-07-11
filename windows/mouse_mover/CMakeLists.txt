cmake_minimum_required(VERSION 3.15)

set(MOUSE_MOVER MouseMover)

set(
    SOURCE_FILES_MMOVER
    mmover.cpp
)
add_executable(
    ${MOUSE_MOVER}
    ${SOURCE_FILES_MMOVER}
)
target_link_libraries(
    ${MOUSE_MOVER}
    ws2_32
)