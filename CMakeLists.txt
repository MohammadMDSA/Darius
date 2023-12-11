cmake_minimum_required (VERSION 3.13)
project(Demo)

set(GAME_PROJECT "Demo")

option(EDITOR_BUILD on)
set(EDITOR_BUILD TRUE)

# Adding Darius Header Tool
add_subdirectory("DariusHeaderTool")

set(ENGINE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Demo")

add_definitions(-D_PROJ_ROOT="${PROJECT_DIR}")
add_definitions(-DGAME_INITIALIZER_DIR="${PROJECT_DIR}/Source/${GAME_PROJECT}/${GAME_PROJECT}.hpp")
add_definitions(-DGAME_NAME=Demo)

# Adding Darius Engine
add_subdirectory("src")

# Adding game project
add_subdirectory("${GAME_PROJECT}/Source")

target_link_libraries("${GAME_PROJECT}" PUBLIC "Darius")
target_include_directories("${GAME_PROJECT}" PUBLIC ${ENGINE_DIR})

if(${EDITOR_BUILD})
target_link_libraries(${GAME_PROJECT} PRIVATE IMGUI)
else(${EDITOR_BUILD})
endif(${EDITOR_BUILD})

add_dependencies("${GAME_PROJECT}" "Darius")