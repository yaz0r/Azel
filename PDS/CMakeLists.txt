cmake_minimum_required(VERSION 3.9)

include_directories(
    "."
    "${CMAKE_SOURCE_DIR}/AzelLib"
    "${THIRD_PARTY}"
    "${THIRD_PARTY}/SDL2/include"
    "${THIRD_PARTY}/imgui"
    "${THIRD_PARTY}/soloud/soloud/include"
    "${THIRD_PARTY}/glm"
    "${THIRD_PARTY}/tracy"
    "${THIRD_PARTY}/bgfx.cmake/bgfx/include"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB PDS_DATA
    "${CMAKE_SOURCE_DIR}/data/*"
)

file(GLOB SRC_ROOT
    "*.cpp"
    "*.h"
)

file(GLOB_RECURSE SRC_KERNEL
    "kernel/*.cpp"
    "kernel/*.h"
)

file(GLOB_RECURSE SRC_FIELD_A3
    "field/field_a3/*.cpp"
    "field/field_a3/*.h"
)

set(SOURCES
 ${SOURCES}
 ${SRC_KERNEL}
 ${SRC_ROOT}
 ${SRC_FIELD_A3}
)

source_group("src" FILES ${SRC_ROOT})
source_group("src\\kernel" FILES ${SRC_KERNEL})
source_group("src\\field\\field_a3" FILES ${SRC_FIELD_A3})

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

set( CMAKE_EXE_LINKER_FLAGS_SHIPPING "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG /OPT:ICF /OPT:REF /SUBSYSTEM:WINDOWS /DEBUG" )

add_executable (PDS ${SOURCES})

IF(APPLE)
IF(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
set ( FRAMEWORKS
        Foundation
        AudioToolbox
        CoreGraphics
        QuartzCore
        UIKit
        OpenGLES
        MobileCoreServices
        AVFoundation
        GameController
	Metal
      )
IF(${CMAKE_SYSTEM_NAME} MATCHES iOS)
set ( FRAMEWORKS
	${FRAMEWORKS}
	CoreMotion
    )
ENDIF()
foreach(FW ${FRAMEWORKS})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework ${FW}")
  endforeach()
ELSE()
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
endif ()
ENDIF (APPLE)

TARGET_LINK_LIBRARIES(PDS ${EXTRA_LIBS})

if(EMSCRIPTEN)
	TARGET_LINK_LIBRARIES(PDS -lGLESv2)
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g0")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=1073741824 -s EXTRA_EXPORTED_RUNTIME_METHODS=['Pointer_stringify'] -s USE_WEBGL2=1 -O3 -g0 --preload-file ${CMAKE_SOURCE_DIR}/data@/")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
 TARGET_LINK_LIBRARIES(PDS soloud AzelLib)
 TARGET_LINK_LIBRARIES(PDS ${THIRD_PARTY}/SDL2/Xcode-iOS/SDL/build/Release\\\${EFFECTIVE_PLATFORM_NAME}/libSDL2.a)
ELSE()
 TARGET_LINK_LIBRARIES(PDS SDL2main SDL2-static soloud AzelLib)
 #TARGET_LINK_LIBRARIES(PDS C:/VulkanSDK/1.1.121.2/Lib/vulkan-1.lib)
 if(UNIX)
  TARGET_LINK_LIBRARIES(PDS -lGL)
 endif()
ENDIF()

TARGET_LINK_LIBRARIES(PDS bgfx)

set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT PDS)
set_target_properties(PDS PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
set_target_properties(PDS PROPERTIES WIN32_EXECUTABLE FALSE)


IF(${CMAKE_SYSTEM_NAME} MATCHES iOS|tvOS)
add_custom_command(TARGET PDS PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
              ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:PDS>
)

set_target_properties(PDS PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/build/ios.plist"
    MACOSX_BUNDLE_BUNDLE_NAME "Azel"
    MACOSX_FRAMEWORK_BUNDLE_VERSION 1.0
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING 1.0 
    MACOSX_BUNDLE_BUNDLE_VERSION 1.0
    MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "net.yaz0r.azel"
)
ENDIF()

