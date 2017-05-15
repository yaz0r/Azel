project(PDS)

cmake_minimum_required(VERSION 3.4)

add_subdirectory(PDS/SDL2)

include_directories("PDS" "PDS/imgui" "PDS/SDL2/include" "PDS/imgui/examples/libs/gl3w" "PDS/imgui/examples/sdl_opengl3_example/")

set(SOURCES
 ${SOURCES}
 PDS/PDS.cpp
 PDS/VDP2.cpp
 PDS/common.cpp
 PDS/dummy.cpp
 PDS/heap.cpp
 PDS/mainMenuDebugTasks.cpp
 PDS/o_title.cpp
 PDS/o_fld_a3.cpp
 PDS/renderer.cpp
 PDS/rootTask.cpp
 PDS/task.cpp
 PDS/titleScreen.cpp
 PDS/3dEngine.cpp
 PDS/field.cpp

 PDS/imgui/imgui.cpp
 PDS/imgui/imgui_draw.cpp
 PDS/imgui/examples/sdl_opengl3_example/imgui_impl_sdl_gl3.cpp
 PDS/imgui/examples/libs/gl3w/GL/gl3w.c
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SDL -std=c++0x")

add_executable (Azel ${SOURCES})
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

TARGET_LINK_LIBRARIES(Azel ${EXTRA_LIBS})
TARGET_LINK_LIBRARIES(Azel SDL2-static)
