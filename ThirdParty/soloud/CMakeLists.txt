cmake_minimum_required(VERSION 3.9)
project(soloud C)

IF(${CMAKE_SYSTEM_NAME} MATCHES iOS)
set(SOLOUD_LIB_NAME soloud-ios)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES tvOS)
Set(SOLOUD_LIB_NAME soloud-tvos)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
Set(SOLOUD_LIB_NAME soloud-darwin)
ELSE()
set(SOLOUD_LIB_NAME soloud)
ENDIF()

file(GLOB_RECURSE SOLOUD_HEADERS ${soloud_SOURCE_DIR}/soloud/include/*.h)
file(GLOB_RECURSE SOLOUD_SOURCES
    ${soloud_SOURCE_DIR}/soloud/src/core/*.cpp
    ${soloud_SOURCE_DIR}/soloud/src/audiosource/wav/*.c*
    ${soloud_SOURCE_DIR}/soloud/src/backend/sdl3_static/*.cpp
    ${soloud_SOURCE_DIR}/soloud/src/backend/null/*.cpp
)

include_directories(${soloud_SOURCE_DIR}/soloud/include "${THIRD_PARTY}/SDL/include/" "${THIRD_PARTY}/SDL/include/SDL3")

set(SOLOUD_SOURCES
${SOLOUD_HEADERS}
${SOLOUD_SOURCES}
)

add_library(${SOLOUD_LIB_NAME} STATIC ${SOLOUD_SOURCES})
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

IF(${CMAKE_SYSTEM_NAME} STREQUAL watchOS)
target_compile_definitions(${SOLOUD_LIB_NAME} PRIVATE WITH_NULL=1)
ELSE()
target_compile_definitions(${SOLOUD_LIB_NAME} PRIVATE WITH_SDL3_STATIC=1)
ENDIF()
