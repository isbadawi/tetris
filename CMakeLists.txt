cmake_minimum_required(VERSION 3.1.0)
project(tetris)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
find_package(SFML COMPONENTS audio graphics system window REQUIRED)

# C++14 for declaring lambda parameters as 'auto'
set(CMAKE_CXX_STANDARD 14)

add_compile_options(-Werror -Wall -Wextra)

set(ASSETS_DIR "${PROJECT_SOURCE_DIR}/assets")
configure_file(include/config.h.in include/config.h)

include_directories("${PROJECT_BINARY_DIR}/include")
include_directories(SYSTEM ${SFML_INCLUDE_DIR})
add_executable(tetris src/tetris.cpp)
target_link_libraries(tetris ${SFML_LIBRARIES})
