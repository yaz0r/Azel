cmake_minimum_required(VERSION 3.9)

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS)
	project(PDS-ios)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES tvOS)
	project(PDS-tvos)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
	project(PDS-Darwin)
ELSE()
	project(PDS)
ENDIF()

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

MACRO(COPY_BUILD_CONFIG SourceConfigName DestConfigName)
if(CMAKE_CONFIGURATION_TYPES)
	list(APPEND CMAKE_CONFIGURATION_TYPES ${DestConfigName})
	string(TOUPPER ${SourceConfigName} SOURCE_UPCASE)
	string(TOUPPER ${DestConfigName} DEST_UPCASE)

	set(CMAKE_SHARED_LINKER_FLAGS_${DEST_UPCASE} ${CMAKE_SHARED_LINKER_FLAGS_${SOURCE_UPCASE}})
	set(CMAKE_EXE_LINKER_FLAGS_${DEST_UPCASE} ${CMAKE_EXE_LINKER_FLAGS_${SOURCE_UPCASE}})
	set(CMAKE_C_FLAGS_${DEST_UPCASE} ${CMAKE_C_FLAGS_${SOURCE_UPCASE}})
	set(CMAKE_CXX_FLAGS_${DEST_UPCASE} ${CMAKE_CXX_FLAGS_${SOURCE_UPCASE}})
else()
	message("  XXX custom build types are not allowed...")
endif()
ENDMACRO(COPY_BUILD_CONFIG)

IF(MSVC)
   if(CMAKE_CONFIGURATION_TYPES)
      list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES MinSizeRel)
      list(REMOVE_ITEM CMAKE_CONFIGURATION_TYPES RelWithDebInfo)
   else()
      message("  XXX custom build types are not allowed...")
   endif()

   #always generate debug info
   set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG" )

   # REF/COMDAT folding
   set( CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /OPT:REF /OPT:ICF" )
   set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF" )

   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /MD /Zc:__cplusplus")

   set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_ITERATOR_DEBUG_LEVEL=1 /D_MSVC_STL_HARDENING=1")
   set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_ITERATOR_DEBUG_LEVEL=1 /D_MSVC_STL_HARDENING=1")

   set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zc:__cplusplus /Zc:preprocessor")
   set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zc:__cplusplus /Zc:preprocessor")

   set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zc:__cplusplus /Zc:preprocessor")
   set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zc:__cplusplus /Zc:preprocessor")

   # for edit and continue
   set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP /ZI /GR-")
   set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /ZI /GR-")

   set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP /Zi /GR-")
   set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP /Zi /GR-")

   COPY_BUILD_CONFIG(Debug Debug_Asan)
   COPY_BUILD_CONFIG(Release Shipping)
   COPY_BUILD_CONFIG(Release Release_Asan)

   # config shipping preprocessors
   set( CMAKE_C_FLAGS_SHIPPING "${CMAKE_C_FLAGS_SHIPPING} -D SHIPPING_BUILD")
   set( CMAKE_CXX_FLAGS_SHIPPING "${CMAKE_CXX_FLAGS_SHIPPING} -D SHIPPING_BUILD")

   COPY_BUILD_CONFIG(Shipping Shipping_Asan)

   # enable asan
   set(CMAKE_CXX_FLAGS_DEBUG_ASAN "${CMAKE_CXX_FLAGS_DEBUG_ASAN} /fsanitize=address /Zi")
   set(CMAKE_CXX_FLAGS_RELEASE_ASAN "${CMAKE_CXX_FLAGS_RELEASE_ASAN} /fsanitize=address /Oy-")
   set(CMAKE_CXX_FLAGS_SHIPPING_ASAN "${CMAKE_CXX_FLAGS_SHIPPING_ASAN} /fsanitize=address /Oy-")
ENDIF(MSVC)

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/ThirdParty")

# BGFX 
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory( ${THIRD_PARTY}/bgfx.cmake )

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
#set(VIDEO_OPENGLES OFF CACHE BOOL "" FORCE)
set(VIDEO_DUMMY OFF CACHE BOOL "" FORCE)
#set(VIDEO_VULKAN OFF CACHE BOOL "" FORCE)
set(RENDER_D3D OFF CACHE BOOL "" FORCE)
set(DISKAUDIO OFF CACHE BOOL "" FORCE)
set(DUMMYAUDIO OFF CACHE BOOL "" FORCE)
set(FORCEFEEDBACK OFF CACHE BOOL "" FORCE)


add_subdirectory( AzelLib )
add_subdirectory( ${THIRD_PARTY}/soloud )

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL watchOS)
ELSEIF(${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
ELSE()
	add_subdirectory( ${THIRD_PARTY}/SDL2 )
	add_subdirectory( PDS )
ENDIF()
