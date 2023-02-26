cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
cmake_policy(SET CMP0076 NEW)

project(imgui-flame-graph)

list(APPEND IMGUI_FLAME_GRAPH_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

add_library(IMGUI_FLAME_GRAPH STATIC)
target_sources(IMGUI_FLAME_GRAPH PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widget_flamegraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widget_flamegraph.h)
target_include_directories(IMGUI_FLAME_GRAPH PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE ${IMGUI_FLAME_GRAPH_DIRECTORIES})

