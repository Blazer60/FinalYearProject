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

set(USE_PRE_BUILT_LIBS OFF)
set(USE_PSEUDO_PCH OFF)
set(ENABLE_PROFILER ON)

message(STATUS "Using Cmake:    " ${CMAKE_VERSION})
message(STATUS "Compiling with: " ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CXX Standard:   " ${CMAKE_CXX_STANDARD})

add_subdirectory(vendor/glm)
add_subdirectory(vendor/glew/build/cmake)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_PLY_IMPORTER ON)
set(ASSIMP_BUILD_BLEND_IMPORTER ON)
set(ASSIMP_BUILD_STL_IMPORTER ON)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/yaml-cpp)

set(HELPER_LIBRARY Statistics)
add_library(${HELPER_LIBRARY} STATIC
        include/helpers/logger/LoggerMacros.h

        src/helpers/logger/Logger.cpp include/helpers/logger/Logger.h
        src/helpers/profiler/ProfileTimer.cpp include/helpers/profiler/ProfileTimer.h
        src/helpers/profiler/Profiler.cpp include/helpers/profiler/Profiler.h
        src/helpers/Timers.cpp include/helpers/Timers.h
        include/helpers/Statistics.h
        src/helpers/FileLoader.cpp
        include/helpers/FileLoader.h
        include/helpers/Format.h
        src/helpers/Format.cpp
)

target_include_directories(${HELPER_LIBRARY} PUBLIC
        include include/helpers include/helpers/logger include/helpers/profiler

        # Vendor Files
        vendor/glm/glm vendor/glew/include/GL
        vendor/glfw/include/GLFW vendor/imgui
        vendor/stb/include vendor/imguizmo
        vendor/assimp/include ${CMAKE_CURRENT_BINARY_DIR}/vendor/assimp/include  # A config file is generated that is required by assimp code.
)

add_executable(${PROJECT_NAME}
        include/Pch.h

        include/engine/Callback.h
        include/engine/EngineMemory.h
        include/engine/loader/RawMesh.h

        include/graphics/CameraSettings.h
        include/graphics/GraphicsDefinitions.h
        include/graphics/GraphicsLighting.h
        include/graphics/buffers/Buffers.h

        src/Main.cpp

        src/engine/Actor.cpp include/engine/Actor.h
        src/engine/Component.cpp include/engine/Component.h
        src/engine/Core.cpp include/engine/Core.h
        src/engine/EngineMath.cpp include/engine/EngineMath.h
        src/engine/EngineState.cpp include/engine/EngineState.h
        src/engine/MainCamera.cpp include/engine/MainCamera.h
        src/engine/MeshComponent.cpp include/engine/MeshComponent.h
        src/engine/Scene.cpp include/engine/Scene.h
        src/engine/event/EventHandler.cpp include/engine/event/EventHandler.h
        src/engine/event/Input.cpp include/engine/event/Input.h
        src/engine/loader/AssimpLoader.cpp include/engine/loader/AssimpLoader.h
        src/engine/loader/CommonLoader.cpp include/engine/loader/CommonLoader.h
        src/engine/loader/FileExplorer.cpp include/engine/loader/FileExplorer.h
        src/engine/loader/MaterialLoader.cpp include/engine/loader/MaterialLoader.h
        src/engine/loader/ModelDestroyer.cpp include/engine/loader/ModelDestroyer.h
        src/engine/loader/ModelLoader.cpp include/engine/loader/ModelLoader.h
        src/engine/loader/TextureLoader.cpp include/engine/loader/TextureLoader.h
        src/engine/rendering/BloomPass.cpp include/engine/rendering/BloomPass.h
        src/engine/rendering/ColourGrading.cpp include/engine/rendering/ColourGrading.h
        src/engine/rendering/Lighting.cpp include/engine/rendering/Lighting.h
        src/engine/ui/Drawable.cpp include/engine/ui/Drawable.h
        src/engine/ui/RendererImGui.cpp include/engine/ui/RendererImGui.h
        src/engine/ui/Ui.cpp include/engine/ui/Ui.h
        src/engine/ui/editor/Editor.cpp include/engine/ui/editor/Editor.h
        src/engine/ui/editor/LogWindow.cpp include/engine/ui/editor/LogWindow.h
        src/engine/ui/editor/ProfilerViewer.cpp include/engine/ui/editor/ProfilerViewer.h
        src/engine/ui/editor/Viewport.cpp include/engine/ui/editor/Viewport.h

        src/game/MyScene.cpp include/game/MyScene.h
        src/game/Rotator.cpp include/game/Rotator.h

        src/graphics/GraphicsDefinitions.cpp
        src/graphics/GraphicsFunctions.cpp include/graphics/GraphicsFunctions.h
        src/graphics/GraphicsState.cpp include/graphics/GraphicsState.h
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

        vendor/imguizmo/ImGuizmo.cpp vendor/imguizmo/ImGuizmo.h
        src/engine/serialize/Serializer.cpp
        include/engine/serialize/Serializer.h
        src/engine/serialize/ComponentSerializer.cpp
        include/engine/serialize/ComponentSerializer.h
        src/engine/loader/SceneLoader.cpp
        include/engine/loader/SceneLoader.h
        src/engine/rendering/MeshRenderer.cpp
        include/engine/rendering/MeshRenderer.h
        src/engine/rendering/MaterialSubComponent.cpp
        include/engine/rendering/MaterialSubComponent.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
        include include/engine include/graphics include/game include/engine/loader include/graphics/buffers
        include/engine/ui include/graphics/postProcessing include/engine/rendering include/helpers/logger
        include/engine/ui/editor include/helpers include/engine/event include/helpers/profiler
        include/engine/serialize

        # Vendor Files
        vendor/glm/glm vendor/glew/include/GL
        vendor/glfw/include/GLFW vendor/imgui
        vendor/stb/include vendor/imguizmo
        vendor/assimp/include ${CMAKE_CURRENT_BINARY_DIR}/vendor/assimp/include  # A config file is generated that is required by assimp code.
        vendor/yaml-cpp/include
        )

if (${USE_PSEUDO_PCH})
    message(STATUS "Using pseudo PCH")
    add_compile_definitions(PCH=1)
else()
    message(STATUS "Using CMake PCH")
    target_precompile_headers(${PROJECT_NAME} PUBLIC
            <iostream> <vector> <unordered_map>
            <string> <string_view> <algorithm>
            <memory> <numeric> <cstdint> <set> <chrono>
            <glm.hpp> <gtc/matrix_transform.hpp> <gtc/type_ptr.hpp>
    )

    target_precompile_headers(${HELPER_LIBRARY} PUBLIC
            <iostream> <vector> <unordered_map>
            <string> <string_view> <algorithm>
            <memory> <numeric> <cstdint> <set> <chrono>
            <glm.hpp> <gtc/matrix_transform.hpp> <gtc/type_ptr.hpp>
    )
endif ()

# STB_IMAGE_IMPLEMENTATION is set within a source file (Texture Loader).
add_compile_definitions(
        GLEW_STATIC
        RESOURCE_PATH="${BINARY_TO_ROOT}/${RESOURCE_PATH}/"
        SHADER_PATH="${BINARY_TO_ROOT}/${SHADER_PATH}/"
        TEXTURE_PATH="${BINARY_TO_ROOT}/${TEXTURE_PATH}/")

if (${ENABLE_PROFILER})
    message(STATUS "Profiling Enabled")
    add_compile_definitions(ENABLE_PROFILING)
endif()

find_package(OpenGL)  # Glew Requires OpenGL to be added.

message(STATUS "Using pre-built libraries: " ${USE_PRE_BUILT_LIBS})
if (${USE_PRE_BUILT_LIBS})
    find_library(GLEW   NAMES libglew32d            PATHS ${CMAKE_SOURCE_DIR}/vendor/lib                REQUIRED)
    find_library(GLFW   NAMES glfw3d glfw3          PATHS ${CMAKE_SOURCE_DIR}/vendor/lib                REQUIRED)
    find_library(IMGUI  NAMES imgui                 PATHS ${CMAKE_SOURCE_DIR}/vendor/lib                REQUIRED)
    find_library(ASSIMP NAMES assimp-vc142-mtd      PATHS ${CMAKE_SOURCE_DIR}/vendor/lib                REQUIRED)
    find_library(ZLIB   NAMES zlibstaticd           PATHS ${CMAKE_SOURCE_DIR}/vendor/lib                REQUIRED)
    find_library(YAML   NAMES yaml-cpp              PATHS ${CMAKE_SOURCE_DIR}/vendor/lib                REQUIRED)

    message(STATUS "    GLEW location: " ${GLEW})
    message(STATUS "    GLFW location: " ${GLFW})
    message(STATUS "   Imgui location: " ${IMGUI})
    message(STATUS "  ASSIMP location: " ${ASSIMP})
    message(STATUS "    zLib location: " ${ZLIB})
    message(STATUS "yaml-cpp location: " ${YAML})

    target_link_libraries(${HELPER_LIBRARY} ${GLEW} OpenGL::GL ${GLFW} ${ASSIMP})
    target_link_libraries(${PROJECT_NAME} ${HELPER_LIBRARY} ${GLEW} ${GLFW} ${IMGUI} OpenGL::GL ${ASSIMP} ${ZLIB} ${YAML})
else ()
    # The names listed are the names given to add_library(). CMake won't tell you if they're correct...
    target_link_libraries(${HELPER_LIBRARY} glew_s OpenGL::GL glfw assimp)
    target_link_libraries(${PROJECT_NAME} ${HELPER_LIBRARY} glew_s glfw imgui OpenGL::GL assimp yaml-cpp)
endif ()

