cmake_minimum_required(VERSION 3.15)

set(INPUTMANIP InputManip)

set(
    SOURCE_FILES_INPUTMANIP
    main.cpp
    helpers.cpp helpers.h)
add_executable(${INPUTMANIP} ${SOURCE_FILES_INPUTMANIP})
