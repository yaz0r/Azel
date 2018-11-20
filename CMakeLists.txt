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

if( MSVC )
	# Eliminate unreferenced functions and data
	# Perform identical COMDAT folding
	set( REL_LINKER_FLAGS "/opt:ref /opt:icf /nodefaultlib:msvcrt" )

    # Make console app
    set( CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /subsystem:console" )

	# String pooling
	# Function-level linking
	# Disable run-time type information
	set( ALL_C_FLAGS "/GF /Gy /GR-" )
	
	# Avoid CRT DLL dependancies in release builds
	set( REL_C_FLAGS "/MT" )

	# Disable warnings for unsecure CRT functions from VC8+
	if( MSVC_VERSION GREATER 1399 )
		set( ALL_C_FLAGS "${ALL_C_FLAGS} /wd4996" )
	endif( MSVC_VERSION GREATER 1399 )
	
	# The CMake configurations set /GR and /MD by default, which conflict with our settings.
	#string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} )
	#string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL} )
	#string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} )
	#string(REPLACE "/MD " " " CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} )
	#string(REPLACE "/MD " " " CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL} )
	#string(REPLACE "/MD " " " CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} )
	#string(REPLACE " /GR" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
endif( MSVC )

set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${REL_LINKER_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} ${REL_LINKER_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${REL_LINKER_FLAGS}" )

set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALL_C_FLAGS}" )
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${REL_C_FLAGS}" )
set( CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${REL_C_FLAGS}" )
set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${REL_C_FLAGS}" )
set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEB_C_FLAGS} -D_DEBUG" )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALL_C_FLAGS}" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${REL_C_FLAGS}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${REL_C_FLAGS}" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${REL_C_FLAGS}" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEB_C_FLAGS} -D_DEBUG" )


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
set_target_properties(Azel PROPERTIES WIN32_EXECUTABLE FALSE)
