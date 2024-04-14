set(FBX_LIBS_SOURCES)
set(FBX_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND FBX_LIBS_INCLUDE
	"FbxLoader.hpp"
	"FBXResource.hpp"
	"FBXPrefabResource.hpp"
	"FBXSystem.hpp"
	"pch.hpp"
	)

list(APPEND FBX_LIBS_SOURCES
	"FbxLoader.cpp"
	"FBXResource.cpp"
	"FBXPrefabResource.cpp"
	"FBXSystem.cpp"
	"pch.cpp"
	)


add_library("Fbx")

target_sources("Fbx"
	PRIVATE
	${FBX_LIBS_SOURCES}
	${FBX_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(BOOST REQUIRED)


set_target_properties("Fbx" PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories("Fbx"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
	${FBX_INCLUDE_DIRS}
)

target_link_libraries("Fbx" PRIVATE ${FBX_LIBS} Renderer Animation Scene IMGUI)


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/FbxTests.cpp" INCLUDE "." LINK Fbx PREFIX Fbx)
endif(BUILD_TESTS)