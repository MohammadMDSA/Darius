set(DEBUG_LIBS_SOURCES)
set(DEBUG_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND DEBUG_LIBS_INCLUDE
	"pch.hpp"
	"DebugDraw.hpp"
	)

list(APPEND DEBUG_LIBS_SOURCES
	"pch.cpp"
	"DebugDraw.cpp"
	)


add_library("Debug")

target_sources("Debug"
	PRIVATE
	${DEBUG_LIBS_SOURCES}
	${DEBUG_LIBS_INCLUDE})

set_target_properties("Debug" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Debug" PRIVATE pch.hpp )
endif()

target_include_directories("Debug"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)

target_link_libraries("Debug" PRIVATE ${Boost_LIBRARIES})
target_link_libraries("Debug" PUBLIC Renderer Graphics ResourceManager Job)


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/DebugTests.cpp" INCLUDE "." LINK Debug PREFIX Debug)
endif(BUILD_TESTS)