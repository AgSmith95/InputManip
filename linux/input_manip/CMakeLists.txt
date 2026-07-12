cmake_minimum_required(VERSION 3.25)

set(INPUTMANIP InputManip)

set(
    SOURCE_FILES_INPUTMANIP
    main.cpp
    ioctl_helpers.hpp
)
add_executable(${INPUTMANIP} ${SOURCE_FILES_INPUTMANIP})
