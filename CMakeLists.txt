cmake_minimum_required(VERSION 3.21)
set(PROJECT_NAME FinalYearProject)

project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 17)

set(USE_PRE_BUILT_LIBS OFF)
set(USE_PSEUDO_PCH OFF)
set(ENABLE_PROFILER ON)

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

set(GRAPHICS_LIBRARY GraphicsOpenGl)
add_library(${GRAPHICS_LIBRARY} STATIC
        include/graphics/CameraSettings.h
        include/graphics/GraphicsLighting.h
        include/graphics/buffers/Buffers.h

        src/graphics/GraphicsDefinitions.cpp include/graphics/GraphicsDefinitions.h
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
)

target_include_directories(${GRAPHICS_LIBRARY} PUBLIC
        include
        include/graphics
        include/graphics/buffers
        include/graphics/postProcessing

        include/helpers
        include/helpers/profiler
        include/helpers/logger

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
        include/engine/loader/SceneLoader.h
        include/engine/rendering/MaterialSubComponent.h
        include/engine/rendering/MeshRenderer.h
        include/engine/serialize/ComponentSerializer.h
        include/engine/serialize/Serializer.h

        src/Main.cpp

        src/engine/Actor.cpp include/engine/Actor.h
        src/engine/Component.cpp include/engine/Component.h
        src/engine/Core.cpp include/engine/Core.h
        src/engine/EngineMath.cpp include/engine/EngineMath.h
        src/engine/EngineState.cpp include/engine/EngineState.h
        src/engine/MainCamera.cpp include/engine/MainCamera.h
        src/engine/Scene.cpp include/engine/Scene.h
        src/engine/event/EventHandler.cpp include/engine/event/EventHandler.h
        src/engine/event/Input.cpp include/engine/event/Input.h
        src/engine/loader/AssimpLoader.cpp include/engine/loader/AssimpLoader.h
        src/engine/loader/CommonLoader.cpp include/engine/loader/CommonLoader.h
        src/engine/loader/FileExplorer.cpp include/engine/loader/FileExplorer.h
        src/engine/loader/MaterialLoader.cpp include/engine/loader/MaterialLoader.h
        src/engine/loader/ModelDestroyer.cpp include/engine/loader/ModelDestroyer.h
        src/engine/loader/ModelLoader.cpp include/engine/loader/ModelLoader.h
        src/engine/loader/SceneLoader.cpp
        src/engine/loader/TextureLoader.cpp include/engine/loader/TextureLoader.h
        src/engine/rendering/BloomPass.cpp include/engine/rendering/BloomPass.h
        src/engine/rendering/ColourGrading.cpp include/engine/rendering/ColourGrading.h
        src/engine/rendering/Lighting.cpp include/engine/rendering/Lighting.h
        src/engine/rendering/MaterialSubComponent.cpp
        src/engine/rendering/MeshRenderer.cpp
        src/engine/serialize/ComponentSerializer.cpp
        src/engine/serialize/Serializer.cpp
        src/engine/ui/Drawable.cpp include/engine/ui/Drawable.h
        src/engine/ui/RendererImGui.cpp include/engine/ui/RendererImGui.h
        src/engine/ui/Ui.cpp include/engine/ui/Ui.h
        src/engine/ui/editor/Editor.cpp include/engine/ui/editor/Editor.h
        src/engine/ui/editor/LogWindow.cpp include/engine/ui/editor/LogWindow.h
        src/engine/ui/editor/ProfilerViewer.cpp include/engine/ui/editor/ProfilerViewer.h
        src/engine/ui/editor/Viewport.cpp include/engine/ui/editor/Viewport.h

        src/game/MyScene.cpp include/game/MyScene.h
        src/game/Rotator.cpp include/game/Rotator.h

        vendor/imguizmo/ImGuizmo.cpp vendor/imguizmo/ImGuizmo.h
        src/engine/ui/editor/ResourceFolder.cpp include/engine/ui/editor/ResourceFolder.h
        src/game/Spammer.cpp src/game/Spammer.h
        src/game/MyTestActor.cpp src/game/MyTestActor.h
        src/engine/loader/ResourcePool.cpp include/engine/loader/ResourcePool.h
        src/engine/loader/ShaderLoader.cpp include/engine/loader/ShaderLoader.h
        src/engine/ui/editor/RendererSettings.cpp src/engine/ui/editor/RendererSettings.h
        src/engine/EngineRandom.cpp include/engine/EngineRandom.h
        src/game/Initialiser.cpp src/game/Initialiser.h
        src/engine/loader/Audio.cpp src/engine/loader/Audio.h
        src/engine/audio/AudioBuffer.cpp include/engine/audio/AudioBuffer.h
        src/engine/audio/AudioSource.cpp include/engine/audio/AudioSource.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
        include include/engine include/graphics include/game include/engine/loader include/graphics/buffers
        include/engine/ui include/graphics/postProcessing include/engine/rendering include/helpers/logger
        include/engine/ui/editor include/helpers include/engine/event include/helpers/profiler
        include/engine/serialize include/engine/audio

        # Vendor Files
        vendor/glm/glm vendor/glew/include/GL
        vendor/glfw/include/GLFW vendor/imgui
        vendor/stb/include vendor/imguizmo
        vendor/assimp/include ${CMAKE_CURRENT_BINARY_DIR}/vendor/assimp/include  # A config file is generated that is required by assimp code.
        vendor/yaml-cpp/include vendor/openal-soft/include
        )

if (${USE_PSEUDO_PCH})
    message(STATUS "Using pseudo PCH")
    add_compile_definitions(GLOBAL_ENGINE_PCH=1)
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

    target_precompile_headers(${GRAPHICS_LIBRARY} PUBLIC
            <iostream> <vector> <unordered_map>
            <string> <string_view> <algorithm>
            <memory> <numeric> <cstdint> <set> <chrono>
            <glm.hpp> <gtc/matrix_transform.hpp> <gtc/type_ptr.hpp>
    )
endif ()

# STB_IMAGE_IMPLEMENTATION is set within a source file (Texture Loader).
add_compile_definitions(
        GLEW_STATIC
        YAML_CPP_STATIC_DEFINE  # Forces YAML to be built statically regardless of cmake settings. see yaml-cpp/dll.h
        )

if (${ENABLE_PROFILER})
    message(STATUS "Profiling Enabled")
    add_compile_definitions(ENABLE_PROFILING)
endif()

find_package(OpenGL)  # Glew Requires OpenGL to be added.

message(STATUS "Using pre-built libraries: " ${USE_PRE_BUILT_LIBS})
if (${USE_PRE_BUILT_LIBS})
    # Assimp is expecting a config.h file that is generated when built from source.
    target_compile_definitions(${HELPER_LIBRARY} PUBLIC USING_PRE_BUILT_LIBS)
    target_include_directories(${HELPER_LIBRARY} PUBLIC vendor/generated/include)
    target_include_directories(${PROJECT_NAME} PUBLIC vendor/generated/include)
    target_include_directories(${GRAPHICS_LIBRARY} PUBLIC vendor/generated/include)

    # Getting information for machine type. Assembling compiler 'triplet' to load correct files.
    string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" COMP)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" PLATFORM)
    string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" ARCH)
    set(COMPILER_TRIPLET "${COMP}-${PLATFORM}-${ARCH}")
    message(STATUS "Compiling for: ${COMPILER_TRIPLET}")
    set(LIB_PATH "${CMAKE_SOURCE_DIR}/vendor/lib/${COMPILER_TRIPLET}")
    message(STATUS "Looking for lib files in: ${LIB_PATH}")
    message(STATUS "Some of the file paths may be wrong as CMake trys to cache variables")

    find_library(GLEW   NAMES glew32d glew32         PATHS ${LIB_PATH} REQUIRED)
    find_library(GLFW   NAMES glfw3d glfw3           PATHS ${LIB_PATH} REQUIRED)
    find_library(IMGUI  NAMES imgui                  PATHS ${LIB_PATH} REQUIRED)
    find_library(ASSIMP NAMES assimp                 PATHS ${LIB_PATH} REQUIRED)
    find_library(ZLIB   NAMES zlibstaticd zlibstatic PATHS ${LIB_PATH} REQUIRED)
    find_library(YAML   NAMES yaml-cpp               PATHS ${LIB_PATH} REQUIRED)

    message(STATUS "    GLEW location: " ${GLEW})
    message(STATUS "    GLFW location: " ${GLFW})
    message(STATUS "   Imgui location: " ${IMGUI})
    message(STATUS "  ASSIMP location: " ${ASSIMP})
    message(STATUS "    zLib location: " ${ZLIB})
    message(STATUS "yaml-cpp location: " ${YAML})

    # The logger knows how to format OpenGl and Assimp errors.
    target_link_libraries(${HELPER_LIBRARY} ${GLEW} OpenGL::GL ${ASSIMP})
    target_link_libraries(${GRAPHICS_LIBRARY} ${GLEW} OpenGL::GL)
    target_link_libraries(${PROJECT_NAME} ${HELPER_LIBRARY} ${GRAPHICS_LIBRARY} ${GLEW} ${GLFW} ${IMGUI} OpenGL::GL ${ASSIMP} ${ZLIB} ${YAML})
else ()
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
    set(YAML_BUILD_SHARED_LIBS OFF)
    add_subdirectory(vendor/yaml-cpp)
    set(LIBTYPE "STATIC")
    add_subdirectory(vendor/openal-soft)

    # The names listed are the names given to add_library(). CMake won't tell you if they're correct...
    target_link_libraries(${HELPER_LIBRARY} glew_s OpenGL::GL assimp)
    target_link_libraries(${GRAPHICS_LIBRARY} glew_s OpenGL::GL)
    target_link_libraries(${PROJECT_NAME} ${HELPER_LIBRARY} ${GRAPHICS_LIBRARY} glew_s glfw imgui OpenGL::GL assimp yaml-cpp OpenAL)
endif ()

