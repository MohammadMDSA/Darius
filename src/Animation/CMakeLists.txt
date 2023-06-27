set(ANIMATION_LIBS_SOURCES)
set(ANIMATION_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND ANIMATION_LIBS_INCLUDE
	"pch.hpp"
	"AnimationResource.hpp"
	"AnimationManager.hpp"
	"AnimationComponent.hpp"
	)

list(APPEND ANIMATION_LIBS_SOURCES
	"pch.cpp"
	"AnimationResource.cpp"
	"AnimationManager.cpp"
	"AnimationComponent.cpp"
	)


add_library("Animation")

target_sources("Animation"
	PRIVATE
	${ANIMATION_LIBS_SOURCES}
	${ANIMATION_LIBS_INCLUDE})

set_target_properties("Animation" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Animation" PRIVATE pch.hpp )
endif()

target_include_directories("Animation"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
	${FBX_INCLUDE_DIRS}
)

target_link_libraries("Animation" PRIVATE ${Boost_LIBRARIES} ${FBX_LINK_LIBS} IMGUI)
target_link_libraries("Animation" PUBLIC Debug Renderer)


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/AnimationTests.cpp" INCLUDE "." LINK Animation PREFIX Animation)
endif(BUILD_TESTS)