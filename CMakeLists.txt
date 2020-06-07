IF(${CMAKE_SYSTEM_NAME} MATCHES iOS)
	project(PDS-ios)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES tvOS)
	project(PDS-tvos)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
	project(PDS-Darwin)
ELSE()
	project(PDS)
ENDIF()

cmake_minimum_required(VERSION 3.9)

MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

message("* Adding build types...")
if(CMAKE_CONFIGURATION_TYPES)
   list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel)
   list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES Release)
   list(APPEND CMAKE_CONFIGURATION_TYPES Shipping)
   list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
       "Add the configurations that we need"
       FORCE)
   message("  Available build types are now : ${CMAKE_CONFIGURATION_TYPES}")
else()
   message("  XXX custom build types are not allowed...")
endif()

set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /GR- /Gl /Gy" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /GR- /Gl /Gy" )

set( CMAKE_C_FLAGS_SHIPPING "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D SHIPPING_BUILD" )
set( CMAKE_CXX_FLAGS_SHIPPING "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -D SHIPPING_BUILD" )

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/ThirdParty")

#set(VULKAN_INCLUDE_PATH "C:/VulkanSDK/1.1.121.2/Include")

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
#set(VIDEO_OPENGLES OFF CACHE BOOL "" FORCE)
set(VIDEO_DUMMY OFF CACHE BOOL "" FORCE)
#set(VIDEO_VULKAN OFF CACHE BOOL "" FORCE)
set(RENDER_D3D OFF CACHE BOOL "" FORCE)
set(DISKAUDIO OFF CACHE BOOL "" FORCE)
set(DUMMYAUDIO OFF CACHE BOOL "" FORCE)
set(FORCEFEEDBACK OFF CACHE BOOL "" FORCE)

add_subdirectory( AzelLib )

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
	add_subdirectory( ${THIRD_PARTY}/soloud )
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL watchOS)
	add_subdirectory( ${THIRD_PARTY}/soloud )
	add_subdirectory( PDS_WatchOS )
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
        add_subdirectory( ${THIRD_PARTY}/soloud )
ELSE()
	add_subdirectory( ${THIRD_PARTY}/SDL2 )
	add_subdirectory( ${THIRD_PARTY}/soloud )
	#add_subdirectory( PDS_Tool )
	add_subdirectory( PDS )
ENDIF()
