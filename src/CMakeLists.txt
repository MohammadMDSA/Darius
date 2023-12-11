cmake_minimum_required (VERSION 3.13)

project ("Darius"
  DESCRIPTION "DirectX 12 3D Game engine and editor"
  LANGUAGES CXX C)

option(BUILD_TEST_TEMPLATE "Ignore warnings related to TODOs" OFF)

option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

option(BUILD_TESTS "Build tests" ON)

option(BUILD_EDITOR "Build target editor" ON)

set(DariusProject "Darius")
set(DariusEngine "Darius")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

set(VCPKG_TOOLCHAIN OFF)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
endif()

if(WINDOWS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WINDOWS)

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
if(NOT WINDOWS)
    if(${CMAKE_BUILD_TYPE} MATCHES Release)
        add_definitions(-DNDEBUG)
    else()
        add_definitions(-D_DEBUG)
    endif()
endif()


set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

if (DEFINED VCPKG_TARGET_ARCHITECTURE)
    set(DIRECTX_ARCH ${VCPKG_TARGET_ARCHITECTURE})
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Ww][Ii][Nn]32$")
    set(DIRECTX_ARCH x86)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Xx]64$")
    set(DIRECTX_ARCH x64)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]$")
    set(DIRECTX_ARCH arm)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]64$")
    set(DIRECTX_ARCH arm64)
endif()

if(BUILD_EDITOR)
    add_definitions(-D_D_EDITOR)
endif(BUILD_EDITOR)

#############################
###### Engine Library #######
#############################

set(DARIUS_ENGINE_LIBS_SOURCES)
set(DARIUS_ENGINE_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND DARIUS_ENGINE_LIBS_INCLUDE
	"${SOURCE_DIR}/Engine/SubsystemRegistry.hpp"
	"${SOURCE_DIR}/Engine/EngineContext.hpp"
	"${SOURCE_DIR}/Engine/pch.hpp"
	)

list(APPEND DARIUS_ENGINE_LIBS_SOURCES	
	"${SOURCE_DIR}/Engine/SubsystemRegistry.cpp"
	"${SOURCE_DIR}/Engine/EngineContext.cpp"
	"${SOURCE_DIR}/Engine/pch.cpp"
	)

add_library(${DariusProject} STATIC)
target_sources(${DariusProject} PRIVATE ${DARIUS_ENGINE_LIBS_SOURCES} ${DARIUS_ENGINE_LIBS_INCLUDE})

target_include_directories(${DariusEngine} PUBLIC ${DARIUS_ENGINE_LIBS_INCLUDE})

set_target_properties(${DariusEngine} PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers(${DariusEngine} PRIVATE ${SOURCE_DIR}/Engine/pch.hpp )
endif()


SET(DARIUS_LIBS)

list(APPEND DARIUS_LIBS
	"Utils"
    "Core"
    "Math"
    "Graphics"
	"Renderer"
    "Scene"
    "ResourceManager"
    "Physics"
    "Debug"
    "Animation"
	"Job"
	)

    	
target_include_directories(${DariusEngine}
PUBLIC
	"${SOURCE_DIR}"
)

##############################
####### 3rd Party Libs #######
##############################

# Adding Flecs
add_subdirectory("${SOURCE_DIR}/Libs/flecs")
target_compile_options(flecs_static PRIVATE)

# Adding imgui
if(EDITOR_BUILD)
add_subdirectory("${SOURCE_DIR}/Libs/imgui_wrapper")
endif(EDITOR_BUILD)

# Adding Memory Allocators
# add_subdirectory("${SOURCE_DIR}/Libs/MemAll")

# Adding FBX SDK
include("../cmake/AddFBXSDK.cmake")
set(BUILD_TOOLS off)
set(BUILD_SAMPLE off)
set(BUILD_DX11 off)

# Adding DirectXTex
add_subdirectory("${SOURCE_DIR}/Libs/DirectXTex")

# Adding Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")

# Adding rttr
add_subdirectory("${SOURCE_DIR}/Libs/rttr")

# Adding enkiTS
set(ENKITS_BUILD_C_INTERFACE OFF)
Set(ENKITS_BUILD_EXAMPLES OFF)
Set(ENKITS_BUILD_SHARED OFF)
Set(ENKITS_INSTALL OFF)
add_subdirectory("${SOURCE_DIR}/Libs/enkiTS")

if(NOT DEFINED BOOST_ROOT)
    set(BOOST_ROOT $ENV{Boost_ROOT})
endif(NOT DEFINED BOOST_ROOT)

find_package(Boost 1.70 COMPONENTS REQUIRED)

if(Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    target_link_libraries(${DariusEngine} PRIVATE ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# Adding DirectXTK 12
add_subdirectory("${SOURCE_DIR}/Libs/DirectXTK12" "${CMAKE_BINARY_DIR}/bin/CMake/DirectXTK12")

# Adding fjs
# add_subdirectory("${SOURCE_DIR}/Libs/fjs")

##############################
############ Libs ############
##############################

SET(DARIUS_LOW_LEVEL_LIBS)
list(APPEND DARIUS_LOW_LEVEL_LIBS
	"Utils"
    "Core"
    "Math"
    "Graphics"
    "Renderer"
    "Scene"
    "ResourceManager"
    "Physics"
    "Debug"
    "Animation"
	"Job"
	)
if(EDITOR_BUILD)
list(APPEND DARIUS_LOW_LEVEL_LIBS
    "Editor")
endif(EDITOR_BUILD)

SET(DARIUS_LIBS ${DARIUS_LOW_LEVEL_LIBS})

foreach(DLib ${DARIUS_LOW_LEVEL_LIBS})
    message("Adding Darius lib as subdirectory ${DLib}")
    add_subdirectory("${SOURCE_DIR}/${DLib}")
    target_link_libraries(${DariusEngine} PUBLIC ${DLib})
endforeach()

##############################
############ TEST ############
##############################

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

if(BUILD_TESTS)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
    enable_testing()
    include(BoostTestHelper)

    add_subdirectory("Tests")
endif(BUILD_TESTS)

##############################
##############################
##############################


SET(LIB_PATHS)
SET(TOTAL_INCLUDES)
foreach(DLib ${DARIUS_LIBS})
    list(APPEND LIB_PATHS
    ${SOURCE_DIR}/${DLib})

    get_target_property(LIB_INCLUDES ${DLib} INCLUDE_DIRECTORIES)
    list(APPEND TOTAL_INCLUDES
    ${LIB_INCLUDES})
endforeach()


# Generating Code before compile
get_target_property(DARIUS_INCLUDES ${DariusEngine} INCLUDE_DIRECTORIES)
list(APPEND TOTAL_INCLUDES
    ${DARIUS_INCLUDES})

FILE(GLOB HPP_SRC_FILES ${SOURCE_DIR}/**/*.hpp)

add_custom_target(DariusGeneratorCommand
                    WORKING_DIRECTORY $<TARGET_FILE_DIR:DariusCodeGenerator>
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SOURCE_DIR}/pch.h" $<TARGET_FILE_DIR:DariusCodeGenerator> && DariusCodeGenerator ${SOURCE_DIR} ${SOURCE_DIR}/Generated "${SOURCE_DIR}/pch.h" ${SOURCE_DIR} ${TOTAL_INCLUDES} && DariusCodeGenerator ${CMAKE_SOURCE_DIR}/Demo/Source/Demo ${CMAKE_SOURCE_DIR}/Demo/Source/Demo/Generated "pch.h" ${TOTAL_INCLUDES}
                    COMMENT "Generating Darius Headers"
                    DEPFILE ${SOURCE_DIR}/)
    

# Defining CMAKE project dependencies
add_dependencies(DariusGeneratorCommand DariusCodeGenerator)
foreach(DLib ${DARIUS_LIBS})
    add_dependencies(${DLib} DariusGeneratorCommand)
    message("Adding generator dependency to project ${DLib}")
endforeach()

add_dependencies(${DariusEngine} DariusGeneratorCommand)