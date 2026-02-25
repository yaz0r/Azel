cmake_minimum_required(VERSION 3.9)

include_directories(
    "."
#    "${THIRD_PARTY}"
    "${THIRD_PARTY}/SDL/include"
    "${THIRD_PARTY}/imgui"
    "${THIRD_PARTY}/imgui/backends"
    "${THIRD_PARTY}/ImGuizmo"
    #"${THIRD_PARTY}/bgfx.cmake/bgfx/3rdparty/dear-imgui"
    "${THIRD_PARTY}/soloud/soloud/include"
    "${THIRD_PARTY}/glm"
    "${THIRD_PARTY}/tracy"
    "${THIRD_PARTY}/bgfx.cmake/bgfx/include"
    "${THIRD_PARTY}/bgfx.cmake/bx/include"
    "${THIRD_PARTY}/bgfx.cmake/bx/include/compat/msvc"
    "${THIRD_PARTY}/bgfx.cmake/bgfx/3rdparty"
    "${THIRD_PARTY}/bgfx.cmake/bimg/include"
    "${THIRD_PARTY}/aosdk"
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
                                        

set(SOURCES
 ${SOURCES}

 ${THIRD_PARTY}/imgui/imgui.cpp
 ${THIRD_PARTY}/imgui/imgui.h
 ${THIRD_PARTY}/imgui/imgui_draw.cpp
 ${THIRD_PARTY}/imgui/imgui_internal.h
 ${THIRD_PARTY}/imgui/imgui_tables.cpp
 ${THIRD_PARTY}/imgui/imgui_widgets.cpp
 ${THIRD_PARTY}/imgui/backends/imgui_impl_sdl3.cpp

 ${THIRD_PARTY}/ImGuizmo/ImGuizmo.cpp
 ${THIRD_PARTY}/ImGuizmo/ImGuizmo.h

 ${THIRD_PARTY}/aosdk/eng_ssf/m68kcpu.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kopac.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kopdm.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kopnz.c
 ${THIRD_PARTY}/aosdk/eng_ssf/m68kops.c
 ${THIRD_PARTY}/aosdk/eng_ssf/sat_hw.c
 ${THIRD_PARTY}/aosdk/eng_ssf/scsp.c
 ${THIRD_PARTY}/aosdk/eng_ssf/scspdsp.c
 #${THIRD_PARTY}/aosdk/eng_ssf/scsplfo.c

 "shaderc_prebuilt.cmake"
)

include(shaderc_prebuilt.cmake)

macro(addShaderProgramPermutation vsname psname varyingname)
    bgfx_compile_shaders(SHADERS shaders/${vsname}.sc AS_HEADERS OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated TYPE VERTEX VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${varyingname}.sc INCLUDE_DIRS ${BGFX_DIR}/src AS_HEADERS)
    bgfx_compile_shaders(SHADERS shaders/${psname}.sc AS_HEADERS OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated TYPE FRAGMENT VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${varyingname}.sc INCLUDE_DIRS ${BGFX_DIR}/src AS_HEADERS)
    #shaderc(FILE shaders/${vsname}.sc BIN2C ${vsname}_${api} OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated/${vsname}_${api}.debug.h DEBUG VERTEX ${platform} VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE ${profileVS})
    #shaderc(FILE shaders/${psname}.sc BIN2C ${psname}_${api} OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated/${psname}_${api}.debug.h DEBUG FRAGMENT ${platform} VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE ${profilePS})
    #set(SOURCES
     #   ${SOURCES}
    #    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated/${vsname}_${api}.h
     #   ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated/${psname}_${api}.h
        #${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated/${vsname}_${api}.debug.h
        #${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated/${psname}_${api}.debug.h
    #)
endmacro()

macro(addShaderProgram vsname psname varyingname)

    set(SOURCES
        ${SOURCES}
        shaders/${vsname}.sc
        shaders/${psname}.sc
        shaders/${varyingname}.sc
    )

    addShaderProgramPermutation(${vsname} ${psname} ${varyingname})
    addShaderProgramPermutation(${vsname} ${psname} ${varyingname})

endmacro()

addShaderProgram(VDP1_vs VDP1_ps VDP1.varying.def)
addShaderProgram(VDP1_vertexColor_vs VDP1_vertexColor_ps VDP1.varying.def)
addShaderProgram(VDP1_2dUI_vs VDP1_2dUI_ps VDP1.varying.def)
addShaderProgram(VDP1_2dUIVertexColor_vs VDP1_2dUIVertexColor_ps VDP1.varying.def)
addShaderProgram(VDP2_vs VDP2_ps VDP2_vs.varying.def)
addShaderProgram(VDP2_blit_vs VDP2_blit_ps VDP2_blit.varying.def)

assign_source_group(${SOURCES})

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS)
add_library (AzelLib-ios ${SOURCES})
target_compile_definitions( AzelLib-ios PUBLIC "BX_CONFIG_DEBUG=0" )
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES tvOS)
add_library (AzelLib-tvos ${SOURCES})
target_compile_definitions( AzelLib-tvos PUBLIC "BX_CONFIG_DEBUG=0" )
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
add_library (AzelLib-darwin ${SOURCES})
target_compile_definitions( AzelLib-darwin PUBLIC "BX_CONFIG_DEBUG=0" )
ELSE()
add_library (AzelLib ${SOURCES})
target_compile_definitions( AzelLib PUBLIC "BX_CONFIG_DEBUG=0" )
ENDIF()
