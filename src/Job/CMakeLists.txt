set(JOB_LIBS_SOURCES)
set(JOB_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND JOB_LIBS_INCLUDE
	"Job.hpp"
	"JobCommon.hpp"
	)

list(APPEND JOB_LIBS_SOURCES
	"Job.cpp"
	)


add_library("Job")

target_sources("Job"
	PRIVATE
	${JOB_LIBS_SOURCES}
	${JOB_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)


set_target_properties("Job" PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories("Job"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)

target_link_libraries("Job" PRIVATE ${Boost_LIBRARIES} enkiTS)
target_link_libraries("Job" PRIVATE Utils Core)


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/JobTests.cpp" INCLUDE "." LINK Job PREFIX Job)
endif(BUILD_TESTS)