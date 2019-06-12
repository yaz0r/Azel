cmake_minimum_required(VERSION 3.9)

include_directories("." "${THIRD_PARTY}/imgui" "${THIRD_PARTY}/SDL2/include" "${THIRD_PARTY}/imgui/examples" "${THIRD_PARTY}/imgui/examples/libs/gl3w" "${THIRD_PARTY}/soloud/soloud/include" "${THIRD_PARTY}/glm")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC_ROOT
    "*.cpp"
    "*.h"
)

file(GLOB_RECURSE SRC_KERNEL
    "kernel/*.cpp"
    "kernel/*.h"
)

file(GLOB SRC_TOWN
    "town/*.cpp"
    "town/*.h"
)

file(GLOB_RECURSE SRC_TOWN_RUIN
    "town/ruin/*.cpp"
    "town/ruin/*.h"
)

file(GLOB_RECURSE SRC_FIELD_A3
    "field/field_a3/*.cpp"
    "field/field_a3/*.h"
)

set(SOURCES
 ${SOURCES}
 ${SRC_KERNEL}
 ${SRC_ROOT}
 ${SRC_TOWN}
 ${SRC_TOWN_RUIN}
 ${SRC_FIELD_A3}

${THIRD_PARTY}/imgui/imgui.cpp
${THIRD_PARTY}/imgui/imgui_draw.cpp
${THIRD_PARTY}/imgui/imgui_widgets.cpp
)

IF(${CMAKE_SYSTEM_NAME} STREQUAL watchOS)
ELSE()
set(SOURCES
 ${SOURCES}
 ${THIRD_PARTY}/imgui/examples/imgui_impl_opengl3.cpp
 ${THIRD_PARTY}/imgui/examples/imgui_impl_sdl.cpp
)
ENDIF()


source_group("src" FILES ${SRC_ROOT})
source_group("src\\kernel" FILES ${SRC_KERNEL})
source_group("src\\town" FILES ${SRC_TOWN})
source_group("src\\town\\ruin" FILES ${SRC_TOWN_RUIN})
source_group("src\\field\\field_a3" FILES ${SRC_FIELD_A3})

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


add_library (AzelLib ${SOURCES})
