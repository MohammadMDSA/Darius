cmake_minimum_required (VERSION 3.13)
project(Demo)

set(GAME_PROJECT "Demo")

add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:NDEBUG>")
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Zi /DEBUG")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VCPKG_TOOLCHAIN OFF)

option(EDITOR_BUILD on)
set(EDITOR_BUILD TRUE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
endif()

if(WINDOWS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WINDOWS)

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
else()
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
endif()


if(BUILD_EDITOR)
    add_definitions(-D_D_EDITOR)
endif(BUILD_EDITOR)

# Adding Darius Header Tool
add_subdirectory("DariusHeaderTool")

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${GAME_PROJECT}")

add_definitions(-D_PROJ_ROOT="${PROJECT_DIR}")
add_definitions(-DGAME_INITIALIZER_DIR="${PROJECT_DIR}/Source/${GAME_PROJECT}/${GAME_PROJECT}.hpp")
add_definitions(-DGAME_NAME=Demo)

# Adding Darius Engine
add_subdirectory("src")

# Adding game project
add_subdirectory("${GAME_PROJECT}/Source")

add_dependencies(${GAME_PROJECT} GameGeneratorCommand)

target_link_libraries("${GAME_PROJECT}" PUBLIC "Darius")
target_include_directories("${GAME_PROJECT}" PUBLIC ${ENGINE_DIR})

if(${EDITOR_BUILD})
target_link_libraries(${GAME_PROJECT} PRIVATE IMGUI)
else(${EDITOR_BUILD})
endif(${EDITOR_BUILD})

add_dependencies("${GAME_PROJECT}" "Darius")