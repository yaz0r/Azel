project(PDS)

cmake_minimum_required(VERSION 3.9)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
#set(VIDEO_OPENGLES OFF CACHE BOOL "" FORCE)
set(VIDEO_DUMMY OFF CACHE BOOL "" FORCE)
set(VIDEO_VULKAN OFF CACHE BOOL "" FORCE)
set(RENDER_D3D OFF CACHE BOOL "" FORCE)
set(DISKAUDIO OFF CACHE BOOL "" FORCE)
set(DUMMYAUDIO OFF CACHE BOOL "" FORCE)
add_subdirectory( PDS/SDL2 )

include_directories("PDS" "PDS/imgui" "PDS/SDL2/include" "PDS/imgui/examples" "PDS/imgui/examples/libs/gl3w")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
 ${SOURCES}

 PDS/3dEngine.cpp
 PDS/3dEngine.h
 PDS/3dEngine_flush.cpp
 PDS/3dEngine_flush.h
 PDS/3dEngine_textureCache.cpp
 PDS/3dEngine_textureCache.h

 PDS/3dModels.h
 PDS/3dModels.cpp

 PDS/common.cpp
 PDS/common_generated.cpp
 PDS/dummy.cpp
 PDS/field.cpp
 PDS/fixedPoint.cpp
 PDS/heap.cpp
 PDS/LCS.cpp
 PDS/LCS.h
 PDS/mainMenuDebugTasks.cpp
 PDS/menu_dragon.cpp
 PDS/menu_dragonMorph.cpp

 PDS/o_fld_a3.cpp
 PDS/o_fld_a3.h
 PDS/o_fld_a3_1.cpp
 PDS/o_fld_a3_1.h
 
 PDS/o_menuEn.cpp
 PDS/o_title.cpp
 PDS/PDS.cpp
 PDS/PDS.h
 PDS/PDS_Logger.cpp
 PDS/PDS_Logger.h
 PDS/renderer.cpp
 PDS/rootTask.cpp
 PDS/task.cpp
 PDS/task.h
 PDS/titleScreen.cpp
 PDS/VDP1.cpp
 PDS/VDP2.cpp
 
 PDS/imgui/imgui.cpp
 PDS/imgui/imgui_draw.cpp
 PDS/imgui/imgui_widgets.cpp
 PDS/imgui/examples/imgui_impl_opengl3.cpp
 PDS/imgui/examples/imgui_impl_sdl.cpp
)

if (MSVC)
    set(SOURCES
        ${SOURCES}
        PDS/imgui/examples/libs/gl3w/GL/gl3w.c
    )
endif()

if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SDL -std=c++0x -D_HAS_EXCEPTIONS=0 /GR-")
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

STRING(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable C++ exceptions
#STRING(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable RTTI

# Always generate debug info
if(MSVC)
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /GR-")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()


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

if(EMSCRIPTEN)
	TARGET_LINK_LIBRARIES(Azel -lGLESv2)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=1073741824 -s DEMANGLE_SUPPORT=1 -s USE_WEBGL2=1 -s USE_SDL=2 --embed-file ${CMAKE_SOURCE_DIR}/data@/ --emrun")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
	TARGET_LINK_LIBRARIES(Azel SDL2main SDL2-static)
endif()

if(UNIX)
	TARGET_LINK_LIBRARIES(Azel -lGL)
endif()


set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT Azel)
set_target_properties(Azel PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
set_target_properties(Azel PROPERTIES WIN32_EXECUTABLE FALSE)
