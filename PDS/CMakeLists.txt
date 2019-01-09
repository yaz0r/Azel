cmake_minimum_required(VERSION 3.9)

include_directories("." "${THIRD_PARTY}/imgui" "${THIRD_PARTY}/SDL2/include" "${THIRD_PARTY}/imgui/examples" "${THIRD_PARTY}/imgui/examples/libs/gl3w")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
 ${SOURCES}

3dEngine.cpp
3dEngine.h
3dEngine_flush.cpp
3dEngine_flush.h
3dEngine_textureCache.cpp
3dEngine_textureCache.h

3dModels.h
3dModels.cpp

kernel/receiveItemTask.h
kernel/receiveItemTask.cpp

common.cpp
common.h
common_generated.cpp
dummy.cpp
field.cpp
field.h
fixedPoint.cpp
heap.cpp
heap.h
LCS.cpp
LCS.h
mainMenuDebugTasks.cpp
mainMenuDebugTasks.h
menu_dragon.cpp
menu_dragonMorph.cpp

o_fld_a3.cpp
o_fld_a3.h
o_fld_a3_1.cpp
o_fld_a3_1.h

o_menuEn.cpp
o_title.cpp
PDS.cpp
PDS.h
PDS_Logger.cpp
PDS_Logger.h
renderer.cpp
rootTask.cpp
task.cpp
task.h
titleScreen.cpp
VDP1.cpp
VDP1.h
VDP2.cpp
VDP2.h

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

if( MSVC )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SDL -std=c++0x -D_HAS_EXCEPTIONS=0 /GR-")
	# Eliminate unreferenced functions and data
	# Perform identical COMDAT folding
	#set( REL_LINKER_FLAGS "/opt:ref /opt:icf /nodefaultlib:msvcrt" )
    #
    ## Make console app
    #set( CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /subsystem:console" )
    #
	## String pooling
	## Function-level linking
	## Disable run-time type information
	#set( ALL_C_FLAGS "/GF /Gy /GR-" )
	#
	## Avoid CRT DLL dependancies in release builds
	#set( REL_C_FLAGS "/MT" )
    #
	## Disable warnings for unsecure CRT functions from VC8+
	#if( MSVC_VERSION GREATER 1399 )
	#	set( ALL_C_FLAGS "${ALL_C_FLAGS} /wd4996" )
	#endif( MSVC_VERSION GREATER 1399 )
	
	# The CMake configurations set /GR and /MD by default, which conflict with our settings.
	#string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE} )
	#string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL} )
	#string(REPLACE "/MD " " " CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} )
	#string(REPLACE "/MD " " " CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} )
	#string(REPLACE "/MD " " " CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL} )
	#string(REPLACE "/MD " " " CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} )
	#string(REPLACE " /GR" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
endif( MSVC )

#set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${REL_LINKER_FLAGS}" )
#set( CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} ${REL_LINKER_FLAGS}" )
#set( CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${REL_LINKER_FLAGS}" )
#
#set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALL_C_FLAGS}" )
#set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${REL_C_FLAGS}" )
#set( CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} ${REL_C_FLAGS}" )
#set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${REL_C_FLAGS}" )
#set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEB_C_FLAGS} -D_DEBUG" )
#
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ALL_C_FLAGS}" )
#set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${REL_C_FLAGS}" )
#set( CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${REL_C_FLAGS}" )
#set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${REL_C_FLAGS}" )
#set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEB_C_FLAGS} -D_DEBUG" )
#
#STRING(REGEX REPLACE "/EH[a-z]+" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable C++ exceptions
#STRING(REGEX REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}) # Disable RTTI

# Always generate debug info
if(MSVC)
  #set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug" )
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /GR-")
else()
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
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
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g3")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=1073741824 -s EXTRA_EXPORTED_RUNTIME_METHODS=['Pointer_stringify'] -s USE_WEBGL2=1 -O3 -g3 --preload-file ${CMAKE_SOURCE_DIR}/data@/")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

TARGET_LINK_LIBRARIES(Azel SDL2main SDL2-static)

if(UNIX)
	TARGET_LINK_LIBRARIES(Azel -lGL)
endif()


set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT Azel)
set_target_properties(Azel PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
set_target_properties(Azel PROPERTIES WIN32_EXECUTABLE FALSE)
