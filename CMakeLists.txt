cmake_minimum_required (VERSION 3.5)

project (ComputeShaders)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17")
set (source_dir "${PROJECT_SOURCE_DIR}/src/")

file (GLOB source_files "${source_dir}/*.cpp")


add_executable (ComputeShaders ${source_files} dependencies/glad/src/glad.c)

add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glm)

target_include_directories(${PROJECT_NAME} 
    PUBLIC dependencies/glfw/include 
    PUBLIC dependencies/glad/include
    PUBLIC dependencies/glm
    PUBLIC src
    )

target_link_directories(${PROJECT_NAME} PRIVATE dependencies/glfw/src PRIVATE dependencies/glad/src)

target_link_libraries(${PROJECT_NAME} glfw)