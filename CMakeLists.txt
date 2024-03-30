cmake_minimum_required (VERSION 3.8)
project(PiratesEngine LANGUAGES C CXX)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set (CMAKE_CXX_STANDARD 20)

include_directories(glad)

add_subdirectory(glfw-3.4)

add_executable(PiratesEngine 
	Engine.cpp 
	glad/glad.c 
 "Engine.h" "Shader.cpp" "Shader.h" )

target_link_libraries(PiratesEngine glfw)