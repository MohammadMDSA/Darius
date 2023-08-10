set(RENDERER_LIBS_SOURCES)
set(RENDERER_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND RENDERER_LIBS_INCLUDE
	"pch.hpp"
	"Camera/CameraManager.hpp"
	"Components/BillboardRendererComponent.hpp"
	"Components/CameraComponent.hpp"
	"Components/IRenderable.hpp"
	"Components/LightComponent.hpp"
	"Components/MeshRendererComponent.hpp"
	"Components/MeshRendererComponentBase.hpp"
	"Components/SkeletalMeshRendererComponent.hpp"
	"Components/TerrainRendererComponent.hpp"
	"Geometry/GeometryGenerator.hpp"
	"Geometry/Mesh.hpp"
	"Geometry/MeshData.hpp"
	"Geometry/ModelLoader/FbxLoader.hpp"
	"Light/LightContext.hpp"
	"Rasterization/Light/ShadowedLightContext.hpp"
	"Rasterization/Renderer.hpp"
	"RayTracing/Pipelines/RayTracingPipeline.hpp"
	"RayTracing/Pipelines/PathTracingPipeline.hpp"
	"RayTracing/Pipelines/SimpleRayTracingRenderer.hpp"
	"RayTracing/RayTracingCommandContext.hpp"
	"RayTracing/RayTracingScene.hpp"
	"RayTracing/Renderer.hpp"
	"RayTracing/Utils/AccelerationStructure.hpp"
	"RayTracing/Utils/ShaderTable.hpp"
	"RayTracing/Utils/Utils.hpp"
	"RendererCommon.hpp"
	"RendererManager.hpp"
	"Resources/BatchResource.hpp"
	"Resources/FBXPrefabResource.hpp"
	"Resources/MaterialResource.hpp"
	"Resources/MeshResource.hpp"
	"Resources/SkeletalMeshResource.hpp"
	"Resources/StaticMeshResource.hpp"
	"Resources/TerrainResource.hpp"
	"Resources/TextureResource.hpp"
	"VertexTypes.hpp"
	)

list(APPEND RENDERER_LIBS_SOURCES
	"pch.cpp"
	"Camera/CameraManager.cpp"
	"Components/BillboardRendererComponent.cpp"
	"Components/CameraComponent.cpp"
	"Components/LightComponent.cpp"
	"Components/MeshRendererComponent.cpp"
	"Components/MeshRendererComponentBase.cpp"
	"Components/SkeletalMeshRendererComponent.cpp"
	"Components/TerrainRendererComponent.cpp"
	"Geometry/GeometryGenerator.cpp"
	"Geometry/Mesh.cpp"
	"Geometry/ModelLoader/FbxLoader.cpp"
	"Light/LightContext.cpp"
	"Rasterization/Renderer.cpp"
	"Rasterization/Light/ShadowedLightContext.cpp"
	"RayTracing/Pipelines/PathTracingPipeline.cpp"
	"RayTracing/Pipelines/SimpleRayTracingRenderer.cpp"
	"RayTracing/RayTracingCommandContext.cpp"
	"RayTracing/RayTracingScene.cpp"
	"RayTracing/Renderer.cpp"
	"RayTracing/Utils/AccelerationStructure.cpp"
	"RayTracing/Utils/ShaderTable.cpp"
	"RendererManager.cpp"
	"Resources/BatchResource.cpp"
	"Resources/FBXPrefabResource.cpp"
	"Resources/MaterialResource.cpp"
	"Resources/MeshResource.cpp"
	"Resources/SkeletalMeshResource.cpp"
	"Resources/StaticMeshResource.cpp"
	"Resources/TerrainResource.cpp"
	"Resources/TextureResource.cpp"
	"VertexTypes.cpp"
	)


add_library("Renderer" STATIC)

target_sources("Renderer"
	PRIVATE
	${RENDERER_LIBS_SOURCES}
	${RENDERER_LIBS_INCLUDE})
	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)

set_target_properties("Renderer" PROPERTIES LINKER_LANGUAGE CXX)


if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Renderer" PRIVATE pch.hpp )
endif()


target_include_directories("Renderer"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
	${FBX_INCLUDE_DIRS}
	"../Libs/DirectXShaderCompiler/inc"
)

include("../../cmake/FindWinPixEventRuntime.cmake")

target_link_libraries("Renderer" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Libs/DirectXShaderCompiler/lib/dxcompiler.lib")

target_link_libraries("Renderer" PRIVATE ${Boost_LIBRARIES} ${FBX_LINK_LIBS} DirectXTex IMGUI PUBLIC Graphics Math Scene)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/RendererTests.cpp" INCLUDE "." LINK Renderer PREFIX Renderer)
endif(BUILD_TESTS)