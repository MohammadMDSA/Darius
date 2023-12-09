set(GAME_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(GAME_PROJECT "Demo")

FILE(GLOB SRC_FILES ${GAME_DIR}/Demo/*.cpp)

set(ENGINE_DIR "C:\Users\AF\source\repos\Darius")
set(SOURCE_DIR ${ENGINE_DIR}\src)

if(${EDITOR_BUILD})
set(EXEC_SRC ${SOURCE_DIR}/Editor/Main.cpp)
else(${EDITOR_BUILD})
endif(${EDITOR_BUILD})

add_executable(${GAME_PROJECT} WIN32
    ${EXEC_SRC}
    ${SOURCE_DIR}/Editor/Main.cpp
    ${SRC_FILES}
    pch.hpp)

add_subdirectory(${ENGINE_DIR})

target_link_libraries(${GAME_PROJECT} PUBLIC DariusEngine)
target_include_directories(${GAME_PROJECT} PUBLIC ${SOURCE_DIR})

if(${EDITOR_BUILD})
target_link_libraries(${GAME_PROJECT} PRIVATE IMGUI)
else(${EDITOR_BUILD})
endif(${EDITOR_BUILD})
    
if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers(${GAME_PROJECT} PRIVATE pch.hpp )
endif()

target_include_directories(${GAME_PROJECT}
                PUBLIC
                ${GAME_DIR}
                ${GAME_DIR}/Demo/Generated)


add_custom_target(GameGeneratorCommand
                    WORKING_DIRECTORY $<TARGET_FILE_DIR:DariusCodeGenerator>
                    COMMAND DariusCodeGenerator ${CMAKE_SOURCE_DIR}/Demo/Source/Demo ${CMAKE_SOURCE_DIR}/Demo/Source/Demo/Generated "pch.h" ${TOTAL_INCLUDES}
                    COMMENT "Generating Game Headers"
                    DEPFILE ${SOURCE_DIR}/)

add_dependencies(${GAME_PROJECT} ${DARIUS_PROJECT})
add_dependencies(${GAME_PROJECT} GameGeneratorCommand)

target_link_libraries(${GAME_PROJECT} PRIVATE
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
    target_compile_options(${GAME_PROJECT} PRIVATE /fp:fast "$<$<NOT:$<CONFIG:DEBUG>>:/guard:cf>")
    target_link_options(${GAME_PROJECT} PRIVATE /DYNAMICBASE /NXCOMPAT)

    if((${CMAKE_SIZEOF_VOID_P} EQUAL 4) AND (NOT ${DIRECTX_ARCH} MATCHES "^arm"))
        target_link_options(${GAME_PROJECT} PRIVATE /SAFESEH)
    endif()
endif()

if(NOT ${DIRECTX_ARCH} MATCHES "^arm")
    if (${CMAKE_SIZEOF_VOID_P} EQUAL "4")
        set(ARCH_SSE2 $<$<CXX_COMPILER_ID:MSVC>:/arch:SSE2> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    else()
        set(ARCH_SSE2 $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-msse2>)
    endif()

    target_compile_options(${GAME_PROJECT} PRIVATE ${ARCH_SSE2})
endif()

if (MINGW OR VCPKG_TOOLCHAIN)
    message("INFO: Using VCPKG for DirectX-Headers and DirectXMath.")
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxmath CONFIG REQUIRED)
    target_link_libraries(${GAME_PROJECT} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids Microsoft::DirectXMath)
    target_compile_definitions(${GAME_PROJECT} PRIVATE USING_DIRECTX_HEADERS)

endif()

if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|IntelLLVM" )
    target_compile_options(${GAME_PROJECT} PRIVATE
        -Wpedantic -Wextra -Werror
        "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic"
        "-Wno-exit-time-destructors" "-Wno-global-constructors" "-Wno-language-extension-token"
        "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro"
        "-Wno-float-equal" "-Wno-switch-enum" "-Wno-tautological-type-limit-compare")

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${GAME_PROJECT} PRIVATE "-Wno-unused-value")
    endif()
endif()
if(MINGW)
    target_compile_options(${GAME_PROJECT} PRIVATE -Wno-ignored-attributes)
    target_link_options(${GAME_PROJECT} PRIVATE -municode)
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    target_compile_options(${GAME_PROJECT} PRIVATE
         /sdl /permissive- /Zc:__cplusplus
         "/wd4061" "/wd4365" "/wd4514" "/wd4571" "/wd4668" "/wd4710" "/wd4820" "/wd5039" "/wd5045"
         "/wd4265" "/wd4625" "/wd4626" "/wd4986" "/wd5204" "/wd5220"
         "/wd4324" "/wd4623" "/wd5262" "/wd5264"
         "/wd5026" "/wd5027" "/wd5038"
         "/wd4464" "/wd4127" "/wd4371" "/wd5205" "/wd5032" "/wd5219" "/wd4868")

    if(ENABLE_CODE_ANALYSIS)
        target_compile_options(${GAME_PROJECT} PRIVATE /analyze)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.24)
        target_compile_options(${GAME_PROJECT} PRIVATE /ZH:SHA_256)
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.26)
        target_compile_options(${GAME_PROJECT} PRIVATE /Zc:preprocessor /wd5105)
    endif()

    if ((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.27) AND (NOT (${DIRECTX_ARCH} MATCHES "^arm")))
        target_link_options(${GAME_PROJECT} PRIVATE /CETCOMPAT)
    endif()

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${GAME_PROJECT} PRIVATE "/wd4555")
    endif()
endif()
if ( CMAKE_CXX_COMPILER_ID MATCHES "^Intel$" )
    target_compile_options(${GAME_PROJECT} PRIVATE /Qwd161)
endif()

if(WIN32)
    target_compile_definitions(${GAME_PROJECT} PRIVATE _UNICODE UNICODE _WIN32_WINNT=0x0A00)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${GAME_PROJECT})


#################################
## Copy dlls and stuff

add_custom_command(TARGET ${GAME_PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "$<TARGET_FILE_DIR:rttr_core>/rttr_core_d.dll"
        $<TARGET_FILE_DIR:${GAME_PROJECT}>)

add_custom_command(TARGET ${GAME_PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/dll"
        $<TARGET_FILE_DIR:${GAME_PROJECT}>)

add_custom_command(TARGET ${GAME_PROJECT} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${GAME_PROJECT}>/Shaders"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/Shaders"
        "$<TARGET_FILE_DIR:${GAME_PROJECT}>/Shaders"

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${GAME_PROJECT}>/EditorResources"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Editor Resources"
        "$<TARGET_FILE_DIR:${GAME_PROJECT}>/EditorResources"

    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${GAME_PROJECT}>/Utils"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Tools/Utils"
        "$<TARGET_FILE_DIR:${GAME_PROJECT}>/Utils")