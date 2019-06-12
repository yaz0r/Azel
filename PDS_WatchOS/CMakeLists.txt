cmake_minimum_required(VERSION 3.9)

include_directories("." "${CMAKE_SOURCE_DIR}/AzelLib" "${THIRD_PARTY}/imgui" "${THIRD_PARTY}/SDL2/include" "${THIRD_PARTY}/imgui/examples" "${THIRD_PARTY}/imgui/examples/libs/gl3w" "${THIRD_PARTY}/soloud/soloud/include" "${THIRD_PARTY}/glm")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC_ROOT
    "*.m"
    "*.cpp"
    "*.h"
)

set(SOURCES
 ${SOURCES}
 ${SRC_ROOT}

${THIRD_PARTY}/imgui/imgui.cpp
${THIRD_PARTY}/imgui/imgui_draw.cpp
${THIRD_PARTY}/imgui/imgui_widgets.cpp
)

add_executable (PDS ${SOURCES})

TARGET_LINK_LIBRARIES(PDS soloud AzelLib)

set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.yaz0r.azel")

