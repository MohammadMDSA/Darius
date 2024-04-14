set(PHYSICS_LIBS_SOURCES)
set(PHYSICS_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND PHYSICS_LIBS_INCLUDE
	"pch.hpp"
	"PhysicsManager.hpp"
	"PhysicsScene.hpp"
	"PhysicsActor.hpp"
	"CollisionCommon.hpp"
	"Components/BoxColliderComponent.hpp"
	"Components/CapsuleColliderComponent.hpp"
	"Components/CharacterControllerComponent.hpp"
	"Components/ColliderComponent.hpp"
	"Components/MeshColliderComponent.hpp"
	"Components/SphereColliderComponent.hpp"
	"Components/RigidbodyComponent.hpp"
	"Resources/PhysicsMaterialResource.hpp"
	)

list(APPEND PHYSICS_LIBS_SOURCES
	"pch.cpp"
	"PhysicsManager.cpp"
	"PhysicsScene.cpp"
	"PhysicsActor.cpp"
 	"Components/BoxColliderComponent.cpp"
	"Components/CapsuleColliderComponent.cpp"
	"Components/CharacterControllerComponent.cpp"
	"Components/ColliderComponent.cpp"
	"Components/MeshColliderComponent.cpp"
 	"Components/SphereColliderComponent.cpp"
	"Components/RigidbodyComponent.cpp"
	"Resources/PhysicsMaterialResource.cpp"
	)


add_library("Physics")

target_sources("Physics"
	PRIVATE
	${PHYSICS_LIBS_SOURCES}
	${PHYSICS_LIBS_INCLUDE})
		
set_target_properties("Physics" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Physics" PRIVATE pch.hpp )
endif()

target_include_directories("Physics"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)

if(${CMAKE_BUILD_TYPE} MATCHES Release)
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/release/PhysX_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/release/PhysXFoundation_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/release/PhysXCommon_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/release/PhysXExtensions_static_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/release/PhysXPvdSDK_static_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/release/PhysXCooking_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/release/PhysXCharacterKinematic_static_64.lib")
else()
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/debug/PhysX_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/debug/PhysXFoundation_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/debug/PhysXCommon_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/debug/PhysXExtensions_static_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/debug/PhysXPvdSDK_static_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/debug/PhysXCooking_64.lib")
	target_link_libraries("Physics" PRIVATE "${SOURCE_DIR}/Libs/PhysX/libs/win.x86_64.vc143/debug/PhysXCharacterKinematic_static_64.lib")
endif()

target_link_libraries("Physics" PRIVATE ${Boost_LIBRARIES} IMGUI)
target_link_libraries("Physics" PUBLIC Job Debug Graphics Renderer)

target_include_directories("Physics" PUBLIC "${SOURCE_DIR}/Libs/PhysX/include")

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/PhysicsTests.cpp" INCLUDE "." LINK Physics PREFIX Physics)
endif(BUILD_TESTS)