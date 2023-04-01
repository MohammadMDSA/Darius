set(DEMO_DIR ${CMAKE_CURRENT_SOURCE_DIR})


FILE(GLOB SRC_FILES ${DEMO_DIR}/Demo/*.cpp)

ADD_LIBRARY(DEMO STATIC
    ${SRC_FILES})

    
set_target_properties(DEMO PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers(DEMO PRIVATE pch.hpp )
endif()

target_include_directories(DEMO
                PUBLIC
                ${DEMO_DIR}
                ${DEMO_DIR}/Demo/Generated)