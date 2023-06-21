cmake_minimum_required(VERSION 3.21)
set(PROJECT_NAME FinalYearProject)

project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)

string(REPLACE ${CMAKE_SOURCE_DIR} "" RELATIVE_PATH ${CMAKE_BINARY_DIR})
string(REGEX MATCHALL / FILE_DIFF ${RELATIVE_PATH})
string(REPLACE "/" "../" BIN_PATH ${FILE_DIFF})
set(BINARY_TO_ROOT  ${BIN_PATH})
set(RESOURCE_PATH   "resources/")
set(SHADER_PATH     "resources/shaders")
set(TEXTURE_PATH    "resources/textures")

set(USE_PRE_BUILT_LIBS ON)
set(USE_PSEUDO_PCH OFF)

message(STATUS "Using Cmake:    " ${CMAKE_VERSION})
message(STATUS "Compiling with: " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CXX Standard:   " ${CMAKE_CXX_STANDARD})

#add_subdirectory(vendor/glm)
#add_subdirectory(vendor/glew/build/cmake)
#add_subdirectory(vendor/glfw)
#add_subdirectory(vendor)

add_executable(${PROJECT_NAME}
        include/Pch.h

        include/graphics/CameraSettings.h
        include/graphics/RendererHelpers.h

        src/Main.cpp
        src/engine/Core.cpp include/engine/Core.h
        src/engine/DebugLogger.cpp include/engine/DebugLogger.h
        src/engine/MainCamera.cpp include/engine/MainCamera.h
        src/engine/Scene.cpp include/engine/Scene.h
        src/engine/Timers.cpp include/engine/Timers.h
        src/engine/loader/CommonLoader.cpp include/engine/loader/CommonLoader.h
        src/engine/loader/MaterialLoader.cpp include/engine/loader/MaterialLoader.h
        src/engine/loader/ModelDestroyer.cpp include/engine/loader/ModelDestroyer.h
        src/engine/loader/ModelLoader.cpp include/engine/loader/ModelLoader.h
        src/engine/loader/RawMesh.cpp include/engine/loader/RawMesh.h
        src/engine/loader/TextureLoader.cpp include/engine/loader/TextureLoader.h

        src/game/MyScene.cpp include/game/MyScene.h

        src/graphics/Materials.cpp include/graphics/Materials.h
        src/graphics/Primitives.cpp src/graphics/Primitives.h
        src/graphics/Renderer.cpp include/graphics/Renderer.h
        src/graphics/Shader.cpp include/graphics/Shader.h
        src/graphics/SubMesh.cpp include/graphics/SubMesh.h
        src/graphics/Vertices.cpp include/graphics/Vertices.h
        src/graphics/WindowHelpers.cpp include/graphics/WindowHelpers.h
        include/engine/loader/MtlMaterialInformation.h)

target_include_directories(${PROJECT_NAME} PUBLIC
        include include/engine include/graphics include/game include/engine/loader

        # Vendor Files
        vendor/glm/glm vendor/glew/include/GL
        vendor/glfw/include/GLFW vendor/imgui
        vendor/stb/include
        )

if (${USE_PSEUDO_PCH})
    message(STATUS "Using pseudo PCH")
    add_compile_definitions(PCH=1)
else()
    message(STATUS "Using CMake PCH")
    target_precompile_headers(${PROJECT_NAME} PUBLIC
            <iostream> <vector> <unordered_map>
            <string> <string_view> <algorithm>
            <memory> <numeric> <cstdint>
            <glm.hpp> [["DebugLogger.h"]])
endif ()

# STB_IMAGE_IMPLEMENTATION is set within a source file (Texture Loader).
add_compile_definitions(
        GLEW_STATIC
        RESOURCE_PATH="${BINARY_TO_ROOT}/${RESOURCE_PATH}/"
        SHADER_PATH="${BINARY_TO_ROOT}/${SHADER_PATH}/"
        TEXTURE_PATH="${BINARY_TO_ROOT}/${TEXTURE_PATH}/")


find_package(OpenGL)  # Glew Requires OpenGL to be added.

if (NOT ${ADD_ECS_TO_EXECUTABLE})
    find_library(ECS NAMES EntityComponentSystem2022 PATHS ${CMAKE_CURRENT_BINARY_DIR}/entity-component-system REQUIRED)
    target_link_libraries(${ECS})
endif ()

if (${USE_PRE_BUILT_LIBS} EQUAL OFF)
    find_library(GLEW  NAMES libglew32d PATHS ${CMAKE_CURRENT_BINARY_DIR}/lib             REQUIRED)
    find_library(GLFW  NAMES glfw3      PATHS ${CMAKE_CURRENT_BINARY_DIR}/vendor/glfw/src REQUIRED)
    find_library(IMGUI NAMES imgui      PATHS ${CMAKE_CURRENT_BINARY_DIR}/vendor          REQUIRED)
else()
    find_library(GLEW  NAMES libglew32d PATHS ${CMAKE_SOURCE_DIR}/vendor/lib REQUIRED)
    find_library(GLFW  NAMES glfw3      PATHS ${CMAKE_SOURCE_DIR}/vendor/lib REQUIRED)
    find_library(IMGUI NAMES imgui      PATHS ${CMAKE_SOURCE_DIR}/vendor/lib REQUIRED)
endif ()

target_link_libraries(${PROJECT_NAME}
        ${GLEW} ${GLFW} ${IMGUI} OpenGL::GL
        )
