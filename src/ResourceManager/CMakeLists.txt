set(RESOUCE_MANAGER_LIBS_SOURCES)
set(RESOUCE_MANAGER_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND RESOUCE_MANAGER_LIBS_INCLUDE
	"ResourceManager.hpp"
	"ResourceTypes/Resource.hpp"
	"ResourceTypes/BatchResource.hpp"
	"ResourceTypes/MeshResource.hpp"
	"ResourceTypes/MaterialResource.hpp"
	"ResourceTypes/TextureResource.hpp"
	"ResourceTypes/SkeletalMeshResource.hpp"
	"ResourceTypes/StaticMeshResource.hpp"
	"ResourceLoader.hpp"
	"pch.hpp"
	)

list(APPEND RESOUCE_MANAGER_LIBS_SOURCES
	"ResourceManager.cpp"
	"ResourceTypes/Resource.cpp"
	"ResourceTypes/MeshResource.cpp"
	"ResourceTypes/BatchResource.cpp"
	"ResourceTypes/MaterialResource.cpp"
	"ResourceTypes/TextureResource.cpp"
	"ResourceTypes/SkeletalMeshResource.cpp"
	"ResourceTypes/StaticMeshResource.cpp"
	"ResourceLoader.cpp"
	"pch.cpp"
	)


add_library("ResourceManager")

target_sources("ResourceManager"
	PRIVATE
	${RESOUCE_MANAGER_LIBS_SOURCES}
	${RESOUCE_MANAGER_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)

set_target_properties("ResourceManager" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("ResourceManager" PRIVATE pch.hpp )
endif()

target_include_directories("ResourceManager"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
	${FBX_INCLUDE_DIRS}
)

target_link_libraries("ResourceManager" PRIVATE ${Boost_LIBRARIES} ${FBX_LINK_LIBS} PUBLIC Core Renderer Utils)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/ResourceManagerTests.cpp" INCLUDE "." LINK ResourceManager PREFIX ResourceManager)
endif(BUILD_TESTS)