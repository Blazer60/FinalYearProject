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
        vendor/imguizmo/ImGuizmo.cpp vendor/imguizmo/ImGuizmo.h

        include/Pch.h

        include/engine/loader/RawMesh.h
        include/engine/logger/LoggerMacros.h

        include/graphics/CameraSettings.h
        include/graphics/GraphicsDefinitions.h
        include/graphics/buffers/Buffers.h

        src/Main.cpp
        src/engine/Actor.cpp include/engine/Actor.h
        src/engine/Component.cpp include/engine/Component.h
        src/engine/Core.cpp include/engine/Core.h
        src/engine/EngineState.cpp include/engine/EngineState.h
        src/engine/MainCamera.cpp include/engine/MainCamera.h
        src/engine/MeshComponent.cpp include/engine/MeshComponent.h
        src/engine/Scene.cpp include/engine/Scene.h
        src/engine/Timers.cpp include/engine/Timers.h
        src/engine/loader/CommonLoader.cpp include/engine/loader/CommonLoader.h
        src/engine/loader/MaterialLoader.cpp include/engine/loader/MaterialLoader.h
        src/engine/loader/ModelDestroyer.cpp include/engine/loader/ModelDestroyer.h
        src/engine/loader/ModelLoader.cpp include/engine/loader/ModelLoader.h
        src/engine/loader/TextureLoader.cpp include/engine/loader/TextureLoader.h
        src/engine/logger/Logger.cpp include/engine/logger/Logger.h
        src/engine/rendering/BloomPass.cpp include/engine/rendering/BloomPass.h
        src/engine/rendering/ColourGrading.cpp include/engine/rendering/ColourGrading.h
        src/engine/ui/Drawable.cpp include/engine/ui/Drawable.h
        src/engine/ui/RendererImGui.cpp include/engine/ui/RendererImGui.h
        src/engine/ui/Ui.cpp include/engine/ui/Ui.h

        src/game/MyScene.cpp include/game/MyScene.h

        src/graphics/GraphicsDefinitions.cpp
        src/graphics/GraphicsFunctions.cpp include/graphics/GraphicsFunctions.h
        src/graphics/GraphicsState.cpp include/graphics/GraphicsState.h
        src/graphics/Lighting.cpp include/graphics/Lighting.h
        src/graphics/Materials.cpp include/graphics/Materials.h
        src/graphics/Mesh.cpp include/graphics/Mesh.h
        src/graphics/Primitives.cpp src/graphics/Primitives.h
        src/graphics/Renderer.cpp include/graphics/Renderer.h
        src/graphics/Shader.cpp include/graphics/Shader.h
        src/graphics/Vertices.cpp include/graphics/Vertices.h
        src/graphics/WindowHelpers.cpp include/graphics/WindowHelpers.h
        src/graphics/buffers/Cubemap.cpp include/graphics/buffers/Cubemap.h
        src/graphics/buffers/FramebufferObject.cpp include/graphics/buffers/FramebufferObject.h
        src/graphics/buffers/HdrTexture.cpp include/graphics/buffers/HdrTexture.h
        src/graphics/buffers/RenderBufferObject.cpp include/graphics/buffers/RenderBufferObject.h
        src/graphics/buffers/Texture.cpp include/graphics/buffers/Texture.h
        src/graphics/buffers/TextureArrayObject.cpp include/graphics/buffers/TextureArrayObject.h
        src/graphics/buffers/TextureBufferObject.cpp include/graphics/buffers/TextureBufferObject.h
        src/graphics/postProcessing/PostProcessLayer.cpp include/graphics/postProcessing/PostProcessLayer.h
        src/engine/ui/editor/Viewport.cpp
        include/engine/ui/editor/Viewport.h
        src/engine/ui/editor/Editor.cpp
        include/engine/ui/editor/Editor.h
        src/engine/EngineMath.cpp include/engine/EngineMath.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
        include include/engine include/graphics include/game include/engine/loader include/graphics/buffers
        include/engine/ui include/graphics/postProcessing include/engine/rendering include/engine/logger
        include/engine/ui/editor

        # Vendor Files
        vendor/glm/glm vendor/glew/include/GL
        vendor/glfw/include/GLFW vendor/imgui
        vendor/stb/include vendor/imguizmo
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
            <glm.hpp> <gtc/matrix_transform.hpp> <gtc/type_ptr.hpp>
            [["Logger.h"]] [["EngineState.h"]] [["LoggerMacros.h"]]
    )
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
