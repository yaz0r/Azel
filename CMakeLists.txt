project(PDS)

cmake_minimum_required(VERSION 3.9)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory( PDS/SDL2 )

include_directories("PDS" "PDS/imgui" "PDS/SDL2/include" "PDS/imgui/examples" "PDS/imgui/examples/libs/gl3w")

set(SOURCES
 ${SOURCES}

 PDS/3dEngine.cpp
 PDS/3dEngine_flush.cpp
 PDS/3dEngine_textureCache.cpp
 PDS/common.cpp
 PDS/common_generated.cpp
 PDS/dummy.cpp
 PDS/field.cpp
 PDS/fixedPoint.cpp
 PDS/heap.cpp
 PDS/mainMenuDebugTasks.cpp
 PDS/menu_dragon.cpp
 PDS/menu_dragonMorph.cpp
 PDS/o_fld_a3.cpp
 PDS/o_menuEn.cpp
 PDS/o_title.cpp
 PDS/PDS.cpp
 PDS/renderer.cpp
 PDS/rootTask.cpp
 PDS/task.cpp
 PDS/titleScreen.cpp
 PDS/VDP1.cpp
 PDS/VDP2.cpp
 
 PDS/imgui/imgui.cpp
 PDS/imgui/imgui_draw.cpp
 PDS/imgui/examples/imgui_impl_opengl3.cpp
 PDS/imgui/examples/imgui_impl_sdl.cpp
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
TARGET_LINK_LIBRARIES(Azel SDL2main SDL2-static)

set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT Azel)
set_target_properties(Azel PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
