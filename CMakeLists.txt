cmake_minimum_required(VERSION 3.1.0)
project(tetris)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_package(SFML COMPONENTS system window graphics REQUIRED)

include_directories(${SFML_INCLUDE_DIR})
add_executable(tetris tetris.cpp)
target_link_libraries(tetris ${SFML_LIBRARIES})
