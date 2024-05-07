set(AUDIO_LIBS_SOURCES)
set(AUDIO_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND AUDIO_LIBS_INCLUDE
	"pch.hpp"
	"AudioManager.hpp"
	"AudioSourceResource.hpp"
	)

list(APPEND AUDIO_LIBS_SOURCES
	"pch.cpp"
	"AudioManager.cpp"
	"AudioSourceResource.cpp"
	)

add_library("Audio")

target_sources("Audio"
	PRIVATE
	${AUDIO_LIBS_SOURCES}
	${AUDIO_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(BOOST REQUIRED)

set_target_properties("Audio" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Audio" PRIVATE pch.hpp )
endif()

target_include_directories("Audio"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)

add_subdirectory("${SOURCE_DIR}/Libs/DirectXTK12" "${CMAKE_BINARY_DIR}/bin/CMake/DirectXTK12")

target_link_libraries("Audio" PUBLIC Scene PRIVATE DirectXTK12 IMGUI)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/AudioTests.cpp" INCLUDE "." LINK Audio PREFIX Audio)
endif(BUILD_TESTS)