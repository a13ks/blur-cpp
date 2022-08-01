cmake_minimum_required(VERSION 3.0)

project(a)

set(SOURCES
    lodepng.cpp
    main.cpp
    blur.h
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++14 -O3 -Wall -Werror -fopenmp)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-fopenmp")

add_subdirectory(doc)
