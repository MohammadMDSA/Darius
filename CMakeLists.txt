cmake_minimum_required (VERSION 3.13)

project ("DariusEngine"
  DESCRIPTION "DirectX 12 3D Game engine and editor"
  LANGUAGES CXX C)

option(BUILD_TEST_TEMPLATE "Ignore warnings related to TODOs" OFF)

option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

option(BUILD_TESTS "Build tests" ON)

option(BUILD_EDITOR "Build target editor" ON)

if(BUILD_EDITOR)
set(PROJECT_NAME "DariusEngine")
endif(BUILD_EDITOR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/CMake")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/CMake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/CMake")

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

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
    add_executable(${PROJECT_NAME} WIN32
        "src/Editor/Editor.cpp"
        "src/Editor/Editor.hpp"
        "src/Editor/Main.cpp"
        "src/Editor/pch.hpp"
        "src/Editor/Camera.hpp"
        "src/Editor/Camera.cpp"
        "src/Editor/EditorContext.hpp"
        "src/Editor/EditorContext.cpp"
        "src/Editor/Simulation.hpp"
        "src/Editor/Simulation.cpp"
        "src/Editor/Gui/GuiManager.hpp"
        "src/Editor/Gui/GuiManager.cpp"
        "src/Editor/Gui/Window.hpp"
        "src/Editor/Gui/Window.cpp"
        "src/Editor/Gui/SceneWindow.hpp"
        "src/Editor/Gui/SceneWindow.cpp"
        "src/Editor/Gui/SceneGraphWindow.hpp"
        "src/Editor/Gui/SceneGraphWindow.cpp"
        "src/Editor/Gui/DetailsWindow.hpp"
        "src/Editor/Gui/DetailsWindow.cpp"
        "src/Editor/Gui/ResourceMonitorWindow.hpp"
        "src/Editor/Gui/ResourceMonitorWindow.cpp"
        "src/Editor/Gui/ProfilerWindow.hpp"
        "src/Editor/Gui/ProfilerWindow.cpp"
        "src/Editor/Gui/Utils/Buffers.hpp"
    )

        add_definitions(-D_D_EDITOR)
        add_definitions(-D_PROJ_ROOT="${CMAKE_SOURCE_DIR}")

    if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
        target_precompile_headers(${PROJECT_NAME} PRIVATE src/Editor/pch.hpp )
    endif()
endif(BUILD_EDITOR)


##############################
####### 3rd Party Libs #######
##############################

# Adding Flecs
add_subdirectory("src/Libs/flecs")
target_compile_options(flecs_static PRIVATE)

# Adding imgui
add_subdirectory("src/Libs/imgui_wrapper")

# Adding Memory Allocators
# add_subdirectory("src/Libs/MemAll")

# Adding FBX SDK
include("cmake/AddFBXSDK.cmake")

# Adding Nlohmann Json
include_directories("src/Libs/nlohmann_json/single_include")

# Adding Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")

if(NOT DEFINED BOOST_ROOT)
    set(BOOST_ROOT $ENV{Boost_ROOT})
endif(NOT DEFINED BOOST_ROOT)

find_package(Boost 1.70 COMPONENTS REQUIRED)

if(Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# Adding DirectXTK 12
add_subdirectory("${SOURCE_DIR}/Libs/DirectXTK12" "${CMAKE_BINARY_DIR}/bin/CMake/DirectXTK12")

# Adding fjs
# add_subdirectory("src/Libs/fjs")

##############################
############ Libs ############
##############################

add_subdirectory("src/utils")
add_subdirectory("src/core")
add_subdirectory("src/math")
add_subdirectory("src/job")
add_subdirectory("src/renderer")
add_subdirectory("src/scene")
add_subdirectory("src/resourceManager")

SET(DARIUS_LIBS)

list(APPEND DARIUS_LIBS
	"Utils"
    "Core"
    "Math"
    "Math"
    "Renderer"
    "Scene"
    "ResourceManager"
	)

target_link_libraries(${PROJECT_NAME} PUBLIC ${DARIUS_LIBS} PRIVATE IMGUIFILEDIALOG)

##############################
############ TEST ############
##############################

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(BUILD_TESTS)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
    enable_testing()
    include(BoostTestHelper)

    add_subdirectory("Tests")
endif(BUILD_TESTS)

##############################
##############################
##############################


##############################
######## Game Project ########
##############################

add_subdirectory("Demo/Source")
target_link_libraries(DEMO PRIVATE Scene IMGUI)
target_include_directories(DEMO PRIVATE "src")
target_link_libraries(${PROJECT_NAME} PRIVATE DEMO)


##############################
##############################
##############################

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d12.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
    runtimeobject.lib
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    "..")

if(MSVC)
    # Use max Warning Level 
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "/W3 " "/Wall " CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
    add_definitions(/WX)
    add_definitions(/MP)
    target_compile_options(${PROJECT_NAME} PRIVATE /fp:fast "$<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>")
    target_link_options(${PROJECT_NAME} PRIVATE /DYNAMICBASE /NXCOMPAT)

    if((${CMAKE_SIZEOF_VOID_P} EQUAL 4) AND (NOT ${DIRECTX_ARCH} MATCHES "^arm"))
        target_link_options(${PROJECT_NAME} PRIVATE /SAFESEH)
    endif()
endif()

if(NOT ${DIRECTX_ARCH} MATCHES "^arm")
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        set(ARCH_SSE2 $<$<CXX_COMPILER_ID:MSVC>:/arch:SSE2> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    else()
        set(ARCH_SSE2 $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE ${ARCH_SSE2})
endif()

if (MINGW OR VCPKG_TOOLCHAIN)
    message("INFO: Using VCPKG for DirectX-Headers and DirectXMath.")
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxmath CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids Microsoft::DirectXMath)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USING_DIRECTX_HEADERS)

endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|IntelLLVM" )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wpedantic -Wextra -Werror
        "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic"
        "-Wno-exit-time-destructors" "-Wno-global-constructors" "-Wno-language-extension-token"
        "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro"
        "-Wno-float-equal" "-Wno-switch-enum" "-Wno-tautological-type-limit-compare")

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unused-value")
    endif()
endif()
if(MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-ignored-attributes)
    target_link_options(${PROJECT_NAME} PRIVATE -municode)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${PROJECT_NAME} PRIVATE
         /sdl /permissive- /Zc:__cplusplus
         "/wd4061" "/wd4365" "/wd4514" "/wd4571" "/wd4668" "/wd4710" "/wd4820" "/wd5039" "/wd5045"
         "/wd4265" "/wd4625" "/wd4626" "/wd4986" "/wd5204" "/wd5220"
         "/wd4324"
         "/wd5026" "/wd5027" "/wd4623" "/wd5038" "/wd4244"
         "/wd4464" "/wd4127" "/wd5205")

    if(ENABLE_CODE_ANALYSIS)
        target_compile_options(${PROJECT_NAME} PRIVATE /analyze)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.24)
        target_compile_options(${PROJECT_NAME} PRIVATE /ZH:SHA_256)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
        target_compile_options(${PROJECT_NAME} PRIVATE /Zc:preprocessor /wd5105)
    endif()

    if ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.27) AND (NOT (${DIRECTX_ARCH} MATCHES "^arm")))
        target_link_options(${PROJECT_NAME} PRIVATE /CETCOMPAT)
    endif()

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${PROJECT_NAME} PRIVATE "/wd4555")
    endif()
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "^Intel$" )
    target_compile_options(${PROJECT_NAME} PRIVATE /Qwd161)
endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE _WIN32_WINNT=0x0A00)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/Shaders"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Shaders"

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/EditorResources"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Editor Resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/EditorResources")