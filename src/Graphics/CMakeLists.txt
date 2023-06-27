set(GRAPHICS_LIBS_SOURCES)
set(GRAPHICS_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND GRAPHICS_LIBS_INCLUDE
	"pch.hpp"
	"AmbientOcclusion/ScreenSpaceAmbientOcclusion.hpp"
	"AntiAliasing/TemporalEffect.hpp"
	"Camera/CameraManager.hpp"
	"CommandContext.hpp"
	"CommandSignature.hpp"
	"Components/BillboardRendererComponent.hpp"
	"Components/CameraComponent.hpp"
	"Components/IRenderable.hpp"
	"Components/LightComponent.hpp"
	"Components/MeshRendererComponent.hpp"
	"Components/MeshRendererComponentBase.hpp"
	"Components/SkeletalMeshRendererComponent.hpp"
	"Components/TerrainRendererComponent.hpp"
	"d3dx12.h"
	"DeviceResources.hpp"
	"FrameResource.hpp"
	"Geometry/GeometryGenerator.hpp"
	"Geometry/Mesh.hpp"
	"Geometry/MeshData.hpp"
	"Geometry/ModelLoader/FbxLoader.hpp"
	"GraphicsCore.hpp"
	"GraphicsUtils/Buffers/ColorBuffer.hpp"
	"GraphicsUtils/Buffers/DepthBuffer.hpp"
	"GraphicsUtils/Buffers/GpuBuffer.hpp"
	"GraphicsUtils/Buffers/PixelBuffer.hpp"
	"GraphicsUtils/Buffers/ReadbackBuffer.hpp"
	"GraphicsUtils/Buffers/ShadowBuffer.hpp"
	"GraphicsUtils/Buffers/Texture.hpp"
	"GraphicsUtils/Buffers/UploadBuffer.hpp"
	"GraphicsUtils/CommandAllocatorPool.hpp"
	"GraphicsUtils/CommandListManager.hpp"
	"GraphicsUtils/D3DUtils.hpp"
	"GraphicsUtils/DDSTextureLoader.hpp"
	"GraphicsUtils/GpuResource.hpp"
	"GraphicsUtils/Memory/DescriptorHeap.hpp"
	"GraphicsUtils/Memory/DynamicDescriptorHeap.hpp"
	"GraphicsUtils/Memory/LinearAllocator.hpp"
	"GraphicsUtils/PipelineState.hpp"
	"GraphicsUtils/Profiling/GpuTimeManager.hpp"
	"GraphicsUtils/Profiling/Profiling.hpp"
	"GraphicsUtils/RootSignature.hpp"
	"GraphicsUtils/SamplerManager.hpp"
	"GraphicsUtils/VertexTypes.hpp"
	"Light/LightManager.hpp"
	"PostProcessing/MotionBlur.hpp"
	"PostProcessing/PostProcessing.hpp"
	"Rasterization/Renderer.hpp"
	"Resources/BatchResource.hpp"
	"Resources/FBXPrefabResource.hpp"
	"Resources/MaterialResource.hpp"
	"Resources/MeshResource.hpp"
	"Resources/SkeletalMeshResource.hpp"
	"Resources/StaticMeshResource.hpp"
	"Resources/TerrainResource.hpp"
	"Resources/TextureResource.hpp"
	)

list(APPEND GRAPHICS_LIBS_SOURCES
	"pch.cpp"
	"AmbientOcclusion/ScreenSpaceAmbientOcclusion.cpp"
	"AntiAliasing/TemporalEffect.cpp"
	"Camera/CameraManager.cpp"
	"CommandContext.cpp"
	"CommandSignature.cpp"
	"Components/BillboardRendererComponent.cpp"
	"Components/CameraComponent.cpp"
	"Components/LightComponent.cpp"
	"Components/MeshRendererComponent.cpp"
	"Components/MeshRendererComponentBase.cpp"
	"Components/SkeletalMeshRendererComponent.cpp"
	"Components/TerrainRendererComponent.cpp"
	"DeviceResources.cpp"
	"Geometry/GeometryGenerator.cpp"
	"Geometry/Mesh.cpp"
	"Geometry/ModelLoader/FbxLoader.cpp"
	"GraphicsCore.cpp"
	"GraphicsUtils/Buffers/ColorBuffer.cpp"
	"GraphicsUtils/Buffers/DepthBuffer.cpp"
	"GraphicsUtils/Buffers/GpuBuffer.cpp"
	"GraphicsUtils/Buffers/PixelBuffer.cpp"
	"GraphicsUtils/Buffers/ReadbackBuffer.cpp"
	"GraphicsUtils/Buffers/ShadowBuffer.cpp"
	"GraphicsUtils/Buffers/Texture.cpp"
	"GraphicsUtils/Buffers/UploadBuffer.cpp"
	"GraphicsUtils/CommandAllocatorPool.cpp"
	"GraphicsUtils/CommandListManager.cpp"
	"GraphicsUtils/D3DUtils.cpp"
	"GraphicsUtils/DDSTextureLoader.cpp"
	"GraphicsUtils/GpuResource.cpp"
	"GraphicsUtils/Memory/DescriptorHeap.cpp"
	"GraphicsUtils/Memory/DynamicDescriptorHeap.cpp"
	"GraphicsUtils/Memory/LinearAllocator.cpp"
	"GraphicsUtils/PipelineState.cpp"
	"GraphicsUtils/Profiling/GpuTimeManager.cpp"
	"GraphicsUtils/Profiling/Profiling.cpp"
	"GraphicsUtils/RootSignature.cpp"
	"GraphicsUtils/SamplerManager.cpp"
	"GraphicsUtils/VertexTypes.cpp"
	"Light/LightManager.cpp"
	"PostProcessing/MotionBlur.cpp"
	"PostProcessing/PostProcessing.cpp"
	"Rasterization/Renderer.cpp"
	"Resources/BatchResource.cpp"
	"Resources/FBXPrefabResource.cpp"
	"Resources/MaterialResource.cpp"
	"Resources/MeshResource.cpp"
	"Resources/SkeletalMeshResource.cpp"
	"Resources/StaticMeshResource.cpp"
	"Resources/TerrainResource.cpp"
	"Resources/TextureResource.cpp"
	)


add_library("Graphics" STATIC)

target_sources("Graphics"
	PRIVATE
	${GRAPHICS_LIBS_SOURCES}
	${GRAPHICS_LIBS_INCLUDE})
	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)

set_target_properties("Graphics" PROPERTIES LINKER_LANGUAGE CXX)


if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Graphics" PRIVATE pch.hpp )
endif()


target_include_directories("Graphics"
PUBLIC
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
	${FBX_INCLUDE_DIRS}
	"../Libs/DirectXShaderCompiler/inc"
)

include("../../cmake/FindWinPixEventRuntime.cmake")

target_link_libraries("Graphics" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Libs/DirectXShaderCompiler/lib/dxcompiler.lib")

target_link_libraries("Graphics" PRIVATE ${Boost_LIBRARIES} ${FBX_LINK_LIBS} DirectXTex IMGUI PUBLIC ResourceManager Scene PIX::WinPixEventRuntime)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/RendererTests.cpp" INCLUDE "." LINK Graphics PREFIX Renderer)
endif(BUILD_TESTS)