cmake_minimum_required (VERSION 3.28.3)
project(PiratesEngine LANGUAGES C CXX)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set (CMAKE_CXX_STANDARD 20)

include_directories(glad)
include_directories(glm)
include_directories(imgui-docking)
include_directories(imgui-docking/backends)

# assimp
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS  OFF CACHE BOOL "ASSIMP WARNINGS AS ERRORS" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Assimp build tests" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Assimp install" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "Assimp install PDB" FORCE)

add_subdirectory(assimp)
#find_package(assimp::assimp)
include_directories( assimp/include/assimp
                   C:/dev/libs/assimp/build/include)

add_subdirectory(glfw-3.4)


add_executable(PiratesEngine "Engine.cpp" "glad/glad.c" "Engine.h" "Renderers/OpenGLRenderer/Shader.cpp" 
"Renderers/OpenGLRenderer/Shader.h" "stb_image.h" "Deps/Camera.h" "Deps/Camera.cpp" "Renderers/OpenGLRenderer/VAO.h" 
"Renderers/OpenGLRenderer/VAO.cpp" "Renderers/OpenGLRenderer/VBO.h" "Renderers/OpenGLRenderer/VBO.cpp" "Renderers/OpenGLRenderer/Texture.cpp" 
"Renderers/OpenGLRenderer/Texture.h"  "imgui-docking/backends/imgui_impl_glfw.cpp" "imgui-docking/backends/imgui_impl_opengl3.cpp" "imgui-docking/imgui.cpp" 
"imgui-docking/imgui_draw.cpp" "imgui-docking/imgui_demo.cpp" "imgui-docking/imgui_tables.cpp" "imgui-docking/imgui_widgets.cpp" "PiratesEngineLoader/gltfLoader.cpp" "PiratesEngineLoader/gltfLoader.h" "json.h" "Renderers/OpenGLRenderer/Mesh.h" "Renderers/OpenGLRenderer/Mesh.cpp" "Renderers/OpenGLRenderer/IBO.h" "Renderers/OpenGLRenderer/IBO.cpp" "PiratesEngineLoader/Model.cpp" "PiratesEngineLoader/Model.h" "Renderers/OpenGLRenderer/AssimpModel.h" "Renderers/OpenGLRenderer/AssimpModel.cpp" "stb_write_image.h" "PCH.hpp")


target_link_libraries(PiratesEngine glfw assimp) #add assimp later
target_precompile_headers(PiratesEngine PRIVATE "PCH.hpp")
set_target_properties(
    PiratesEngine PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
