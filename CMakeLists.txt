cmake_minimum_required (VERSION 3.20)

project (ComputeShaders)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17")
set (source_dir "${PROJECT_SOURCE_DIR}/src/")


file (GLOB source_files "${source_dir}/*.cpp")
file (GLOB imgui_src "dependencies/imgui/*.cpp")

add_executable (ComputeShaders ${source_files} 
                ${imgui_src}
                dependencies/glad/src/glad.c
                dependencies/stb_image/stb_image.cpp
                )

add_subdirectory(dependencies/glfw)
add_subdirectory(dependencies/glm)

target_include_directories(${PROJECT_NAME} 
    PUBLIC dependencies/glfw/include 
    PUBLIC dependencies/glad/include
    PUBLIC dependencies/glm
    PUBLIC dependencies/imgui
    PUBLIC dependencies/stb_image
    PUBLIC src
    )

target_link_directories(${PROJECT_NAME} PRIVATE dependencies/glfw/src PRIVATE dependencies/glad/src PRIVATE dependencies/imgui)

target_link_libraries(${PROJECT_NAME} glfw)