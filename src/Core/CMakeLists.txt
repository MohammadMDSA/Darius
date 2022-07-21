set(CORE_LIBS_SOURCES)
set(CORE_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND CORE_LIBS_INCLUDE
	"TimeManager/StepTimer.hpp"
	"TimeManager/TimeManager.hpp"
    "Signal.hpp"
	"Memory.hpp"
	"Exception.hpp"
	"Ref.hpp"
	"Containers/ConcurrentQueue.hpp"
	)

list(APPEND CORE_LIBS_SOURCES
	"TimeManager/TimeManager.cpp"
	"Signal.cpp"
	"Memory.cpp"
	)


add_library("Core")

target_sources("Core"
	PRIVATE
	${CORE_LIBS_SOURCES}
	${CORE_LIBS_INCLUDE} "pch.hpp")

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)

set_target_properties("Core" PROPERTIES LINKER_LANGUAGE CXX)


if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Core" PRIVATE pch.hpp )
endif()


target_include_directories("Core"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)

target_link_libraries("Core" PRIVATE ${Boost_LIBRARIES})

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	add_boost_test(SOURCE "Tests/CoreTests.cpp" INCLUDE "." LINK Core PREFIX Core)
endif(BUILD_TESTS)