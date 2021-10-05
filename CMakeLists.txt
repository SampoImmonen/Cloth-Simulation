cmake_minimum_required (VERSION 3.20)

project (ComputeShaders)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17")
set (source_dir "${PROJECT_SOURCE_DIR}/src/")

find_package(imgui CONFIG)

file (GLOB source_files "${source_dir}/*.cpp")


add_executable (ComputeShaders ${source_files} 
                dependencies/imgui/imgui_impl_glfw.cpp
                dependencies/imgui/imgui_impl_opengl3.cpp
                dependencies/imgui/imgui_impl_glfw.h
                dependencies/imgui/imgui_impl_opengl3.h
                dependencies/glad/src/glad.c)

add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glm)

target_include_directories(${PROJECT_NAME} 
    PUBLIC dependencies/glfw/include 
    PUBLIC dependencies/glad/include
    PUBLIC dependencies/glm
    PUBLIC dependencies/imgui
    PUBLIC src
    )

target_link_directories(${PROJECT_NAME} PRIVATE dependencies/glfw/src PRIVATE dependencies/glad/src)

target_link_libraries(${PROJECT_NAME} glfw)