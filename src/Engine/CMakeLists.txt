set(DARIUS_ENGINE_LIBS_SOURCES)
set(DARIUS_ENGINE_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND DARIUS_ENGINE_LIBS_INCLUDE
	"SubsystemRegistry.hpp"
	"EngineContext.hpp"
	"pch.hpp"
	)

list(APPEND DARIUS_ENGINE_LIBS_SOURCES	
	"SubsystemRegistry.cpp"
	"EngineContext.cpp"
	"pch.cpp"
	)


add_library("DariusEngine")

target_sources("DariusEngine"
	PRIVATE
	${DARIUS_ENGINE_LIBS_SOURCES}
	${DARIUS_ENGINE_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)


set_target_properties("DariusEngine" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("DariusEngine" PRIVATE pch.hpp )
endif()


##############################
############ Libs ############
##############################

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

	
target_include_directories("DariusEngine"
PUBLIC
	".."
)

target_link_libraries("DariusEngine" PUBLIC "${DARIUS_LIBS}")
target_link_libraries("DariusEngine" PRIVATE ${Boost_LIBRARIES})


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/DariusEngineTests.cpp" INCLUDE "." LINK DariusEngine PREFIX DariusEngine)
endif(BUILD_TESTS)