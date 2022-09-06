set(UTILS_LIBS_SOURCES)
set(UTILS_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND UTILS_LIBS_INCLUDE
    "Assert.hpp"
	"Common.hpp"
	"Log.hpp"
	"Debug.hpp"
	"Detailed.hpp"
	)
	
list(APPEND UTILS_LIBS_SOURCES
	"Assert.cpp"
	"Debug.cpp"
	)


add_library("Utils")

target_sources("Utils"
	PRIVATE 
	${UTILS_LIBS_SOURCES}
	${UTILS_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)

set_target_properties("Utils" PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories("Utils"
PUBLIC
	".."
PRIVATE
	${Boost_INCLUDE_DIRS}
)

target_link_libraries("Utils" PRIVATE ${Boost_LIBRARIES})

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	#add_boost_test(SOURCE "Tests/UtilsTests.cpp" INCLUDE "." LINK Utils PREFIX Utils)
endif(BUILD_TESTS)