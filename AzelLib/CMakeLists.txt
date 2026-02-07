cmake_minimum_required(VERSION 3.9)

include_directories(
    "."
#    "${THIRD_PARTY}"
    "${CMAKE_BINARY_DIR}/ThirdParty/SDL2/include-config-debug"
    "${CMAKE_BINARY_DIR}/ThirdParty/SDL2/include"
    "${THIRD_PARTY}/SDL2/include"
    "${THIRD_PARTY}/imgui"
    "${THIRD_PARTY}/ImGuizmo"
    #"${THIRD_PARTY}/bgfx.cmake/bgfx/3rdparty/dear-imgui"
    "${THIRD_PARTY}/soloud/soloud/include"
    "${THIRD_PARTY}/glm"
    "${THIRD_PARTY}/tracy"
    "${THIRD_PARTY}/bgfx.cmake/bgfx/include"
    "${THIRD_PARTY}/bgfx.cmake/bx/include"
    "$<$<CXX_COMPILER_ID:MSVC>:${THIRD_PARTY}/bgfx.cmake/bx/include/compat/msvc>"
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
 ${THIRD_PARTY}/imgui/imgui_widgets.cpp
 ${THIRD_PARTY}/imgui/examples/imgui_impl_sdl.cpp

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

if(UNIX AND NOT APPLE)
    set( SHADERC_COMMAND "${CMAKE_BINARY_DIR}/ThirdParty/bgfx.cmake/shaderc" )
else()
    set( SHADERC_COMMAND "${THIRD_PARTY}/../vs2022_tools/Release/shaderc" )
endif()

include(shaderc_prebuilt.cmake)

macro(addShaderProgram vsname psname varyingname)

set(SOURCES
    ${SOURCES}
    shaders/${vsname}.sc
    shaders/${psname}.sc
    shaders/${varyingname}.sc
)

if(UNIX AND NOT APPLE)
    # GLSL 150 is the max supported by old BGFX shaderc (1.10-1.50 only)
    # The 704-pixel resolution fix may help more than shader version
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.glsl.bin VERTEX LINUX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE 150)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.glsl.bin FRAGMENT LINUX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE 150)
    # SPIRV shaders for Vulkan backend
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.spirv.bin VERTEX LINUX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE spirv)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.spirv.bin FRAGMENT LINUX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE spirv)
else()
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.glsl.bin VERTEX WINDOWS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE 430)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.glsl.bin FRAGMENT WINDOWS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE 430)
endif()

set(SOURCES
    ${SOURCES}
    ${THIRD_PARTY}/../data/shaders/generated/${vsname}.glsl.bin
    ${THIRD_PARTY}/../data/shaders/generated/${psname}.glsl.bin
)

if(UNIX AND NOT APPLE)
    set(SOURCES
        ${SOURCES}
        ${THIRD_PARTY}/../data/shaders/generated/${vsname}.spirv.bin
        ${THIRD_PARTY}/../data/shaders/generated/${psname}.spirv.bin
    )
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.metal.bin VERTEX IOS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.metal.bin FRAGMENT IOS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
    set(SOURCES
        ${SOURCES}
        ${THIRD_PARTY}/../data/shaders/generated/${vsname}.metal.bin
        ${THIRD_PARTY}/../data/shaders/generated/${psname}.metal.bin
    )
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.metal.bin VERTEX OSX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.metal.bin FRAGMENT OSX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE metal)
    set(SOURCES
        ${SOURCES}
        ${THIRD_PARTY}/../data/shaders/generated/${vsname}.metal.bin
        ${THIRD_PARTY}/../data/shaders/generated/${psname}.metal.bin
    )
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.dx11.bin VERTEX WINDOWS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE vs_5_0)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.dx11.bin FRAGMENT WINDOWS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE ps_5_0)
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.dx11.debug.bin DEBUG VERTEX WINDOWS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE vs_5_0)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.dx11.debug.bin DEBUG FRAGMENT WINDOWS VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE ps_5_0)
    set(SOURCES
        ${SOURCES}
        ${THIRD_PARTY}/../data/shaders/generated/${vsname}.dx11.bin
        ${THIRD_PARTY}/../data/shaders/generated/${psname}.dx11.bin
        ${THIRD_PARTY}/../data/shaders/generated/${vsname}.dx11.debug.bin
        ${THIRD_PARTY}/../data/shaders/generated/${psname}.dx11.debug.bin
    )

    
    shaderc(FILE shaders/${vsname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${vsname}.spirv.bin VERTEX LINUX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE spirv)
    shaderc(FILE shaders/${psname}.sc OUTPUT ${THIRD_PARTY}/../data/shaders/generated/${psname}.spirv.bin FRAGMENT LINUX VARYINGDEF shaders/${varyingname}.sc INCLUDES ${BGFX_DIR}/src PROFILE spirv)
    set(SOURCES
        ${SOURCES}
        ${THIRD_PARTY}/../data/shaders/generated/${vsname}.spirv.bin
        ${THIRD_PARTY}/../data/shaders/generated/${psname}.spirv.bin
    )
endif()

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
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES tvOS)
add_library (AzelLib-tvos ${SOURCES})
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
add_library (AzelLib-darwin ${SOURCES})
ELSE()
add_library (AzelLib ${SOURCES})
if(UNIX AND NOT APPLE)
    add_dependencies(AzelLib shaderc)
endif()
ENDIF()
