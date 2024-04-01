cmake_minimum_required (VERSION 3.8)
project(PiratesEngine LANGUAGES C CXX)

include(FetchContent)

FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)
set(CMAKE_USE_RELATIVE_PATHS true)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_BINARY_DIR})

include_directories(glad)

add_subdirectory(glfw-3.4)


add_executable(PiratesEngine 
	Engine.cpp 
	glad/glad.c 
 "Engine.h" "Shader.cpp" "Shader.h" "stb_image.h")

target_link_libraries(PiratesEngine PRIVATE glfw glm::glm)
set_target_properties(
    PiratesEngine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
