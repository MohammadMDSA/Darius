cmake_minimum_required (VERSION 3.13)

project ("Darius"
  DESCRIPTION "DirectX 12 3D Game engine and editor"
  LANGUAGES CXX C)

option(BUILD_TEST_TEMPLATE "Ignore warnings related to TODOs" OFF)

option(ENABLE_CODE_ANALYSIS "Use Static Code Analysis on build" OFF)

option(BUILD_TESTS "Build tests" ON)

option(BUILD_EDITOR "Build target editor" ON)

if(BUILD_EDITOR)
set(PROJECT_NAME "Darius")
endif(BUILD_EDITOR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")

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
        "src/Editor/Gui/ThumbnailManager.hpp"
        "src/Editor/Gui/ThumbnailManager.cpp"
        "src/Editor/Gui/Windows/Window.hpp"
        "src/Editor/Gui/Windows/Window.cpp"
        "src/Editor/Gui/Windows/ContentWindow.hpp"
        "src/Editor/Gui/Windows/ContentWindow.cpp"
        "src/Editor/Gui/Windows/DetailsWindow.hpp"
        "src/Editor/Gui/Windows/DetailsWindow.cpp"
        "src/Editor/Gui/Windows/GameWindow.hpp"
        "src/Editor/Gui/Windows/GameWindow.cpp"
        "src/Editor/Gui/Windows/ProfilerWindow.hpp"
        "src/Editor/Gui/Windows/ProfilerWindow.cpp"
        "src/Editor/Gui/Windows/ResourceMonitorWindow.hpp"
        "src/Editor/Gui/Windows/ResourceMonitorWindow.cpp"
        "src/Editor/Gui/Windows/SceneGraphWindow.hpp"
        "src/Editor/Gui/Windows/SceneGraphWindow.cpp"
        "src/Editor/Gui/Windows/SceneWindow.hpp"
        "src/Editor/Gui/Windows/SceneWindow.cpp"
        "src/Editor/Gui/Windows/SettingsWindow.hpp"
        "src/Editor/Gui/Windows/SettingsWindow.cpp"
        "src/Editor/Gui/Utils/Buffers.hpp"
        "src/Editor/Gui/Components/ContentWindowComponents.cpp"
        "src/Editor/Gui/Components/ContentWindowComponents.hpp"
        "src/Editor/Gui/Components/Common.cpp"
        "src/Editor/Gui/Components/Common.hpp"
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
include_directories(SYSTEM "src/Libs/flecs")

# Adding imgui
add_subdirectory("src/Libs/imgui_wrapper")
target_link_libraries(${PROJECT_NAME} PRIVATE IMGUI_FLAME_GRAPH IMGUIFILEDIALOG)

# Adding Memory Allocators
# add_subdirectory("src/Libs/MemAll")

# Adding FBX SDK
include("cmake/AddFBXSDK.cmake")
set(BUILD_TOOLS off)
set(BUILD_SAMPLE off)
set(BUILD_DX11 off)

# Adding Nlohmann Json
include_directories(SYSTEM "src/Libs/nlohmann_json/single_include")

# Adding DirectXTex
add_subdirectory("src/Libs/DirectXTex")

# Adding Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")

# Adding PhysX Includes
include_directories("src/Libs/PhysX/include")

# Adding rttr
add_subdirectory("src/Libs/rttr")

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

# Adding Darius Header Tool
add_subdirectory("${CMAKE_SOURCE_DIR}/DariusHeaderTool")

# Adding fjs
# add_subdirectory("src/Libs/fjs")

##############################
############ Libs ############
##############################

SET(DARIUS_LOW_LEVEL_LIBS)
list(APPEND DARIUS_LOW_LEVEL_LIBS
	"Utils"
    "Core"
    "Math"
    "Renderer"
    "Scene"
    "ResourceManager"
    "Physics"
    "Debug"
    "Animation"
	"Job"
	)

SET(DARIUS_LIBS ${DARIUS_LOW_LEVEL_LIBS})
list(APPEND DARIUS_LIBS
    "DariusEngine")

foreach(DLib ${DARIUS_LOW_LEVEL_LIBS})
message("Adding Darius lib as subdirectory ${DLib}")
add_subdirectory("src/${DLib}")
endforeach()

add_subdirectory("src/Engine")

target_link_libraries(${PROJECT_NAME} PUBLIC DariusEngine)

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
target_link_libraries(DEMO PUBLIC DariusEngine PRIVATE IMGUI)
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
         "/wd4324" "/wd4623"
         "/wd5026" "/wd5027" "/wd5038"
         "/wd4464" "/wd4127" "/wd4371" "/wd5205" "/wd5032" "/wd5219" "/wd4868")

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
get_target_property(DARIUS_INCLUDES ${PROJECT_NAME} INCLUDE_DIRECTORIES)
list(APPEND TOTAL_INCLUDES
    ${DARIUS_INCLUDES})

add_custom_target(DariusGeneratorCommand
                    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                    COMMAND DariusCodeGenerator ${SOURCE_DIR} ${SOURCE_DIR}/Generated ${SOURCE_DIR}/Renderer/pch.hpp ${SOURCE_DIR} ${TOTAL_INCLUDES} && DariusCodeGenerator ${CMAKE_SOURCE_DIR}/Demo/Source/Demo ${CMAKE_SOURCE_DIR}/Demo/Source/Demo/Generated ${SOURCE_DIR}/Renderer/pch.hpp ${TOTAL_INCLUDES}
                    COMMENT "Generating Darius Headers")

# Defining CMAKE project dependencies
    
add_dependencies(DariusGeneratorCommand DariusCodeGenerator)
foreach(DLib ${DARIUS_LIBS})
    add_dependencies(${DLib} DariusGeneratorCommand)
    message("Adding generator dependency to project ${DLib}")
endforeach()

add_dependencies(DEMO DariusGeneratorCommand)
add_dependencies(${PROJECT_NAME} DariusGeneratorCommand)


# Copying rttr dll to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "$<TARGET_FILE_DIR:rttr_core>/rttr_core_d.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

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
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/EditorResources"

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Utils"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Tools/Utils"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Utils")