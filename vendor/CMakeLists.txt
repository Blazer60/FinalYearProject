option(IMGUI_BUILD_FOR_OPENGL3  "Build Imgui for OpenGl3 or greater"     ON)
option(IMGUI_BUILD_FOR_GLFW3    "Build Imgui for glfw3"                  ON)

set(IMGUI_LIBRARY_NAME imgui)

set(IMGUI_ROOT_DIRECTORY        ${CMAKE_CURRENT_LIST_DIR}/imgui)
set(IMGUI_BACKEND_DIRECTORY     ${IMGUI_ROOT_DIRECTORY}/backends)


set(IMGUI_BACKEND_SRC       "")
set(IMGUI_BACKEND_HEADER    "")

if (${IMGUI_BUILD_FOR_OPENGL3})
    list(APPEND IMGUI_BACKEND_SRC       ${IMGUI_BACKEND_DIRECTORY}/imgui_impl_opengl3.cpp)
    list(APPEND IMGUI_BACKEND_HEADER    ${IMGUI_BACKEND_DIRECTORY}/imgui_impl_opengl3.h)
endif ()
if (${IMGUI_BUILD_FOR_GLFW3})
    list(APPEND IMGUI_BACKEND_SRC       ${IMGUI_BACKEND_DIRECTORY}/imgui_impl_glfw.cpp)
    list(APPEND IMGUI_BACKEND_HEADER    ${IMGUI_BACKEND_DIRECTORY}/imgui_impl_glfw.h)
endif ()

add_library(${IMGUI_LIBRARY_NAME} STATIC
        ${IMGUI_ROOT_DIRECTORY}/imgui.h             ${IMGUI_ROOT_DIRECTORY}/imgui.cpp
        ${IMGUI_ROOT_DIRECTORY}/imgui_demo.cpp      ${IMGUI_ROOT_DIRECTORY}/imgui_draw.cpp
        ${IMGUI_ROOT_DIRECTORY}/imgui_tables.cpp    ${IMGUI_ROOT_DIRECTORY}/imgui_widgets.cpp

        ${IMGUI_BACKEND_SRC}                        ${IMGUI_BACKEND_HEADER}
        )

target_include_directories(${IMGUI_LIBRARY_NAME} PUBLIC
        ${IMGUI_ROOT_DIRECTORY}                     ${IMGUI_BACKEND_DIRECTORY}
        ${CMAKE_CURRENT_LIST_DIR}/glfw/include
        )
