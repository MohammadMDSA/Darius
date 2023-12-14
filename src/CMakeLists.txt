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
set(FLECS_STATIC FALSE)
add_subdirectory("${SOURCE_DIR}/Libs/flecs")

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


##############################
##############################
##############################


if(MSVC)
    # Use max Warning Level 
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    add_definitions(/WX)
    add_definitions(/MP)
    target_compile_options(${DariusEngine} PRIVATE /fp:fast "$<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>")
    target_link_options(${DariusEngine} PRIVATE /DYNAMICBASE /NXCOMPAT)

    if((${CMAKE_SIZEOF_VOID_P} EQUAL 4) AND (NOT ${DIRECTX_ARCH} MATCHES "^arm"))
        target_link_options(${DariusEngine} PRIVATE /SAFESEH)
    endif()
endif()

if(NOT ${DIRECTX_ARCH} MATCHES "^arm")
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        set(ARCH_SSE2 $<$<CXX_COMPILER_ID:MSVC>:/arch:SSE2> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    else()
        set(ARCH_SSE2 $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    endif()

    target_compile_options(${DariusEngine} PRIVATE ${ARCH_SSE2})
endif()

if (MINGW OR VCPKG_TOOLCHAIN)
    message("INFO: Using VCPKG for DirectX-Headers and DirectXMath.")
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxmath CONFIG REQUIRED)
    target_link_libraries(${DariusEngine} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids Microsoft::DirectXMath)
    target_compile_definitions(${DariusEngine} PRIVATE USING_DIRECTX_HEADERS)

endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|IntelLLVM" )
    target_compile_options(${DariusEngine} PRIVATE
        -Wpedantic -Wextra -Werror
        "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic"
        "-Wno-exit-time-destructors" "-Wno-global-constructors" "-Wno-language-extension-token"
        "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro"
        "-Wno-float-equal" "-Wno-switch-enum" "-Wno-tautological-type-limit-compare")

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${DariusEngine} PRIVATE "-Wno-unused-value")
    endif()
endif()
if(MINGW)
    target_compile_options(${DariusEngine} PRIVATE -Wno-ignored-attributes)
    target_link_options(${DariusEngine} PRIVATE -municode)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${DariusEngine} PRIVATE
         /sdl /permissive- /Zc:__cplusplus
         "/wd4061" "/wd4365" "/wd4514" "/wd4571" "/wd4668" "/wd4710" "/wd4820" "/wd5039" "/wd5045"
         "/wd4265" "/wd4625" "/wd4626" "/wd4986" "/wd5204" "/wd5220"
         "/wd4324" "/wd4623" "/wd5262" "/wd5264"
         "/wd5026" "/wd5027" "/wd5038"
         "/wd4464" "/wd4127" "/wd4371" "/wd5205" "/wd5032" "/wd5219" "/wd4868" "/wd5267")

    if(ENABLE_CODE_ANALYSIS)
        target_compile_options(${DariusEngine} PRIVATE /analyze)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.24)
        target_compile_options(${DariusEngine} PRIVATE /ZH:SHA_256)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
        target_compile_options(${DariusEngine} PRIVATE /Zc:preprocessor /wd5105)
    endif()

    if ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.27) AND (NOT (${DIRECTX_ARCH} MATCHES "^arm")))
        target_link_options(${DariusEngine} PRIVATE /CETCOMPAT)
    endif()

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${DariusEngine} PRIVATE "/wd4555")
    endif()
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "^Intel$" )
    target_compile_options(${DariusEngine} PRIVATE /Qwd161)
endif()

if(WIN32)
    target_compile_definitions(${DariusEngine} PRIVATE _UNICODE UNICODE _WIN32_WINNT=0x0A00)
endif()

##############################
##############################
##############################


add_custom_target(DariusGeneratorCommand
                    WORKING_DIRECTORY $<TARGET_FILE_DIR:DariusCodeGenerator>
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SOURCE_DIR}/pch.h" $<TARGET_FILE_DIR:DariusCodeGenerator> && DariusCodeGenerator ${SOURCE_DIR} ${SOURCE_DIR}/Generated "${SOURCE_DIR}/pch.h" ${SOURCE_DIR} ${TOTAL_INCLUDES} && DariusCodeGenerator ${CMAKE_SOURCE_DIR}/Demo/Source/Demo ${CMAKE_SOURCE_DIR}/Demo/Source/Demo/Generated "pch.h" ${TOTAL_INCLUDES}
                    COMMENT "Generating Darius Headers"
                    DEPFILE ${SOURCE_DIR}/)
    

add_custom_target(GameGeneratorCommand
                    WORKING_DIRECTORY $<TARGET_FILE_DIR:DariusCodeGenerator>
                    COMMAND DariusCodeGenerator ${PROJECT_DIR}/Source/${GAME_PROJECT} ${PROJECT_DIR}/Source/${GAME_PROJECT}/Generated "pch.h" ${TOTAL_INCLUDES}
                    COMMENT "Generating Game Headers"
                    DEPFILE ${GAME_DIR}/${DariusEngine})

# Defining CMAKE project dependencies
add_dependencies(DariusGeneratorCommand DariusCodeGenerator)
foreach(DLib ${DARIUS_LIBS})
    add_dependencies(${DLib} DariusGeneratorCommand)
    message("Adding generator dependency to project ${DLib}")
endforeach()

add_dependencies(${DariusEngine} DariusGeneratorCommand)