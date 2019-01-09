cmake_minimum_required(VERSION 3.9)

include_directories("." "${CMAKE_SOURCE_DIR}/PDS" "${THIRD_PARTY}/imgui" "${THIRD_PARTY}/SDL2/include" "${THIRD_PARTY}/imgui/examples" "${THIRD_PARTY}/imgui/examples/libs/gl3w")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
 ${SOURCES}

PDS_Tool.cpp

${THIRD_PARTY}/imgui/imgui.cpp
${THIRD_PARTY}/imgui/imgui_draw.cpp
${THIRD_PARTY}/imgui/imgui_widgets.cpp
${THIRD_PARTY}/imgui/examples/imgui_impl_opengl3.cpp
${THIRD_PARTY}/imgui/examples/imgui_impl_sdl.cpp
)

if (MSVC)
    set(SOURCES
        ${SOURCES}
        ${THIRD_PARTY}/imgui/examples/libs/gl3w/GL/gl3w.c
    )
endif()


add_executable (PDS_Tool ${SOURCES})

IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
   FIND_LIBRARY(OpenAL_LIBRARY OpenAL )
   FIND_LIBRARY(AVFoundation_LIBRARY AVFoundation )
   MARK_AS_ADVANCED (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY
                     OpenAL_LIBRARY
                     AVFoundation_LIBRARY )
   SET(EXTRA_LIBS ${SDL2_LIBRARY} ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY} ${OpenAL_LIBRARY} ${AVFoundation_LIBRARY} )
ENDIF (APPLE)

TARGET_LINK_LIBRARIES(PDS_Tool ${EXTRA_LIBS})

if(EMSCRIPTEN)
	TARGET_LINK_LIBRARIES(PDS_Tool -lGLESv2)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=1073741824 -s -s USE_WEBGL2=1 -s USE_SDL=2 --llvm-lto 3 -O3 -g0 --preload-file ${CMAKE_SOURCE_DIR}/data@/")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
	TARGET_LINK_LIBRARIES(PDS_Tool SDL2main SDL2-static)
endif()

if(UNIX)
	TARGET_LINK_LIBRARIES(PDS_Tool -lGL)
endif()


set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT PDS_Tool)
set_target_properties(PDS_Tool PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
set_target_properties(PDS_Tool PROPERTIES WIN32_EXECUTABLE FALSE)
