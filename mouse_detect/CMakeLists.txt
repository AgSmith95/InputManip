cmake_minimum_required(VERSION 3.15)

set(MOUSE_DETECTOR_EXE MouseDetector)
set(MOUSE_DETECTOR_DLL MouseDetectorDLL)

set(
    SOURCE_FILES_MDETECTOR_DLL
    MouseDetectorDLL.c
)
add_library(
    ${MOUSE_DETECTOR_DLL} SHARED
    ${SOURCE_FILES_MDETECTOR_DLL}
)
# set_target_properties(
#     ${MOUSE_DETECTOR_DLL} PROPERTIES
#     LINKER_LANGUAGE C
# )
target_link_libraries(
    ${MOUSE_DETECTOR_DLL}
    ws2_32
)

set(
    SOURCE_FILES_MDETECTOR
    mdetect.cpp
)
add_executable(
    ${MOUSE_DETECTOR_EXE}
    ${SOURCE_FILES_MDETECTOR}
)
set_target_properties(${MOUSE_DETECTOR_EXE} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_link_libraries(
    ${MOUSE_DETECTOR_EXE}
    ws2_32
)
add_dependencies(
    ${MOUSE_DETECTOR_EXE}
    ${MOUSE_DETECTOR_DLL}
)
