set(SCENE_LIBS_SOURCES)
set(SCENE_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND SCENE_LIBS_INCLUDE
	"GameObject.hpp"
	"Scene.hpp"
    "Utils/DetailsDrawer.hpp"
	"Utils/Serializer.hpp"
	"SceneLight.hpp"
	"EntityComponentSystem/Entity.hpp"
	"EntityComponentSystem/Components/ComponentBase.hpp"
	"EntityComponentSystem/Components/TransformComponent.hpp"
	"EntityComponentSystem/Components/MeshRendererComponent.hpp"
	"EntityComponentSystem/Components/SkeletalMeshRendererComponent.hpp"
	"EntityComponentSystem/Components/LightComponent.hpp"
	"EntityComponentSystem/Components/BehaviourComponent.hpp"
	"pch.hpp"
	)

list(APPEND SCENE_LIBS_SOURCES
	"GameObject.cpp"
	"Scene.cpp"
	"pch.cpp"
    "Utils/Utils.cpp"
    "Utils/DetailsDrawer.cpp"
	"Utils/Serializer.cpp"
	"EntityComponentSystem/Components/ComponentBase.cpp"
	"EntityComponentSystem/Components/TransformComponent.cpp"
	"EntityComponentSystem/Components/MeshRendererComponent.cpp"
	"EntityComponentSystem/Components/SkeletalMeshRendererComponent.cpp"
	"EntityComponentSystem/Components/LightComponent.cpp"
	"EntityComponentSystem/Components/BehaviourComponent.cpp"
	)


add_library("Scene")

target_sources("Scene"
	PRIVATE
	${SCENE_LIBS_SOURCES}
	${SCENE_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)

set_target_properties("Scene" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Scene" PRIVATE pch.hpp )
endif()

target_include_directories("Scene"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)


target_link_libraries("Scene" PRIVATE ${Boost_LIBRARIES} IMGUI)
target_link_libraries("Scene" PUBLIC flecs_static Utils Core Math Renderer ResourceManager Job Debug)


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/SceneTests.cpp" INCLUDE "." LINK Scene PREFIX Scene)
endif(BUILD_TESTS)