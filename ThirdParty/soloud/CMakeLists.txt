cmake_minimum_required(VERSION 2.8.11)
project(soloud C)

file(GLOB_RECURSE SOLOUD_HEADERS ${soloud_SOURCE_DIR}/soloud/include/*.h)
file(GLOB_RECURSE SOLOUD_SOURCES_CORE ${soloud_SOURCE_DIR}/soloud/src/core/*.cpp ${soloud_SOURCE_DIR}/soloud/src/backend/sdl2_static/*.cpp)
#list(REMOVE_ITEM SOLOUD_SOURCES ${soloud_SOURCE_DIR}/soloud/src/backend/vita_homebrew/soloud_vita_homebrew.cpp)

include_directories(${soloud_SOURCE_DIR}/soloud/include "${THIRD_PARTY}/SDL2/include")

set(SOLOUD_SOURCES
${SOLOUD_HEADERS}
${SOLOUD_SOURCES_CORE}
)

add_library(soloud STATIC ${SOLOUD_SOURCES})

target_compile_definitions(soloud PRIVATE WITH_SDL2_STATIC=1)
