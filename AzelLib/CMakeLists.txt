cmake_minimum_required(VERSION 3.9)

include_directories(
    "."
    "${THIRD_PARTY}/imgui"
    "${THIRD_PARTY}/SDL2/include"
    "${THIRD_PARTY}/imgui/examples"
    "${THIRD_PARTY}/imgui/examples/libs/gl3w"
    "${THIRD_PARTY}/soloud/soloud/include"
    "${THIRD_PARTY}/glm"
    "${THIRD_PARTY}/tracy"
    "${THIRD_PARTY}/aosdk"
    "${THIRD_PARTY}/bgfx.cmake/bgfx/include"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC_ROOT
    "*.cpp"
    "*.h"
)

set(SOURCES
 ${SOURCES}
 ${SRC_ROOT}
)

function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group(${SOURCES})


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

if (MSVC)
ADD_MSVC_PRECOMPILED_HEADER("PDS.h" "PDS_Precompiled.cpp" SOURCES)

SET_SOURCE_FILES_PROPERTIES("PDS_Precompiled.cpp"
                            PROPERTIES COMPILE_FLAGS "/Yc\"PDS.h\" /FI\"PDS.h\" /Fp\"PDS.pch\""
                                        OBJECT_DEPENDS "PDS.pch")
endif()
                                        
if (MSVC)
    set(SOURCES
        ${SOURCES}
        ${THIRD_PARTY}/imgui/examples/libs/gl3w/GL/gl3w.c
    )
endif()

set(SOURCES
 ${SOURCES}
 ${THIRD_PARTY}/imgui/imgui.cpp
 ${THIRD_PARTY}/imgui/imgui_draw.cpp
 ${THIRD_PARTY}/imgui/imgui_widgets.cpp

 ${THIRD_PARTY}/aosdk/eng_ssf/m68kcpu.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kopac.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kopdm.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kopnz.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kops.c
 ${THIRD_PARTY}/aosdk/eng_ssf/sat_hw.c
 ${THIRD_PARTY}/aosdk/eng_ssf/scsp.c
 ${THIRD_PARTY}/aosdk/eng_ssf/scspdsp.c
 #${THIRD_PARTY}/aosdk/eng_ssf/scsplfo.c

 ${THIRD_PARTY}/../data/VDP2_vs.glsl
 ${THIRD_PARTY}/../data/VDP2_vs.sc
 ${THIRD_PARTY}/../data/VDP2_ps.glsl
 ${THIRD_PARTY}/../data/VDP2_ps.sc
)

shaderc(FILE ${THIRD_PARTY}/../data/VDP2_vs.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_vs.glsl_bin DEBUG VERTEX WINDOWS VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE 430)
shaderc(FILE ${THIRD_PARTY}/../data/VDP2_ps.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_ps.glsl_bin DEBUG FRAGMENT WINDOWS VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE 430)
set(SOURCES
${SOURCES}
${THIRD_PARTY}/../data/VDP2_vs.glsl_bin
${THIRD_PARTY}/../data/VDP2_ps.glsl_bin
)

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
shaderc(FILE ${THIRD_PARTY}/../data/VDP2_vs.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_vs.metal_bin DEBUG VERTEX IOS VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
shaderc(FILE ${THIRD_PARTY}/../data/VDP2_ps.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_ps.metal_bin DEBUG FRAGMENT IOS VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
set(SOURCES
${SOURCES}
${THIRD_PARTY}/../data/VDP2_vs.metal_bin
${THIRD_PARTY}/../data/VDP2_ps.metal_bin
)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
shaderc(FILE ${THIRD_PARTY}/../data/VDP2_vs.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_vs.metal_bin DEBUG VERTEX OSX VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
shaderc(FILE ${THIRD_PARTY}/../data/VDP2_ps.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_ps.metal_bin DEBUG FRAGMENT OSX VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
set(SOURCES
${SOURCES}
${THIRD_PARTY}/../data/VDP2_vs.metal_bin
${THIRD_PARTY}/../data/VDP2_ps.metal_bin
)
else()
shaderc(FILE ${THIRD_PARTY}/../data/VDP2_vs.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_vs.dx11_bin DEBUG VERTEX WINDOWS VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE vs_5_0)
shaderc(FILE ${THIRD_PARTY}/../data/VDP2_ps.sc OUTPUT ${THIRD_PARTY}/../data/VDP2_ps.dx11_bin DEBUG FRAGMENT WINDOWS VARYINGDEF ${THIRD_PARTY}/../data/VDP2_vs.varying.def.sc INCLUDES ${BGFX_DIR}/src PROFILE ps_5_0)

set(SOURCES
${SOURCES}
${THIRD_PARTY}/../data/VDP2_vs.dx11_bin
${THIRD_PARTY}/../data/VDP2_ps.dx11_bin
)
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS)
add_library (AzelLib-ios ${SOURCES})
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES tvOS)
add_library (AzelLib-tvos ${SOURCES})
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
add_library (AzelLib-darwin ${SOURCES})
ELSE()
add_library (AzelLib ${SOURCES})
ENDIF()
