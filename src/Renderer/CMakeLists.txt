set(RENDERER_LIBS_SOURCES)
set(RENDERER_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND RENDERER_LIBS_INCLUDE
	"pch.hpp"
	"AntiAliasing/TemporalEffect.hpp"
	"AmbientOcclusion/ScreenSpaceAmbientOcclusion.hpp"
	"GraphicsUtils/D3DUtils.hpp"
	"GraphicsUtils/VertexTypes.hpp"
	"GraphicsUtils/GpuResource.hpp"
	"GraphicsUtils/CommandListManager.hpp"
	"GraphicsUtils/CommandAllocatorPool.hpp"
	"GraphicsUtils/RootSignature.hpp"
	"GraphicsUtils/PipelineState.hpp"
	"GraphicsUtils/SamplerManager.hpp"
	"GraphicsUtils/DDSTextureLoader.hpp"
	"GraphicsUtils/Profiling/Profiling.hpp"
	"GraphicsUtils/Profiling/GpuTimeManager.hpp"
	"GraphicsUtils/Memory/DescriptorHeap.hpp"
	"GraphicsUtils/Memory/LinearAllocator.hpp"
	"GraphicsUtils/Memory/DynamicDescriptorHeap.hpp"
	"GraphicsUtils/Buffers/PixelBuffer.hpp"
	"GraphicsUtils/Buffers/ColorBuffer.hpp"
	"GraphicsUtils/Buffers/GpuBuffer.hpp"
	"GraphicsUtils/Buffers/UploadBuffer.hpp"
	"GraphicsUtils/Buffers/ShadowBuffer.hpp"
	"GraphicsUtils/Buffers/DepthBuffer.hpp"
	"GraphicsUtils/Buffers/ReadbackBuffer.hpp"
	"GraphicsUtils/Buffers/Texture.hpp"
	"Light/LightManager.hpp"
	"PostProcessing/MotionBlur.hpp"
	"PostProcessing/PostProcessing.hpp"
	"Resources/BatchResource.hpp"
	"Resources/MeshResource.hpp"
	"Resources/MaterialResource.hpp"
	"Resources/TextureResource.hpp"
	"Resources/SkeletalMeshResource.hpp"
	"Resources/StaticMeshResource.hpp"
	"Resources/TerrainResource.hpp"
	"Camera/CameraManager.hpp"
	"Components/CameraComponent.hpp"
	"Components/BillboardRendererComponent.hpp"
	"Components/IRenderable.hpp"
	"Components/LightComponent.hpp"
	"Components/MeshRendererComponentBase.hpp"
	"Components/MeshRendererComponent.hpp"
	"Components/SkeletalMeshRendererComponent.hpp"
	"Components/TerrainRendererComponent.hpp"
	"CommandSignature.hpp"
	"CommandContext.hpp"
	"Geometry/ModelLoader/FbxLoader.hpp"
	"Geometry/Mesh.hpp"
	"Geometry/MeshData.hpp"
	"Geometry/GeometryGenerator.hpp"
	"Renderer.hpp"
	"GraphicsCore.hpp"
	"DeviceResources.hpp"
	"FrameResource.hpp"
	"d3dx12.h"
	)

list(APPEND RENDERER_LIBS_SOURCES
	"pch.cpp"
	"AntiAliasing/TemporalEffect.cpp"
	"AmbientOcclusion/ScreenSpaceAmbientOcclusion.cpp"
	"DeviceResources.cpp"
	"GraphicsCore.cpp"
	"GraphicsUtils/D3DUtils.cpp"
	"GraphicsUtils/VertexTypes.cpp"
	"GraphicsUtils/GpuResource.cpp"
	"GraphicsUtils/CommandListManager.cpp"
	"GraphicsUtils/CommandAllocatorPool.cpp"
	"GraphicsUtils/RootSignature.cpp"
	"GraphicsUtils/PipelineState.cpp"
	"GraphicsUtils/SamplerManager.cpp"
	"GraphicsUtils/DDSTextureLoader.cpp"
	"GraphicsUtils/Profiling/Profiling.cpp"
	"GraphicsUtils/Profiling/GpuTimeManager.cpp"
	"GraphicsUtils/Memory/DescriptorHeap.cpp"
	"GraphicsUtils/Memory/LinearAllocator.cpp"
	"GraphicsUtils/Memory/DynamicDescriptorHeap.cpp"
	"GraphicsUtils/Buffers/PixelBuffer.cpp"
	"GraphicsUtils/Buffers/ColorBuffer.cpp"
	"GraphicsUtils/Buffers/GpuBuffer.cpp"
	"GraphicsUtils/Buffers/UploadBuffer.cpp"
	"GraphicsUtils/Buffers/DepthBuffer.cpp"
	"GraphicsUtils/Buffers/ShadowBuffer.cpp"
	"GraphicsUtils/Buffers/ReadbackBuffer.cpp"
	"GraphicsUtils/Buffers/Texture.cpp"
	"Light/LightManager.cpp"
	"PostProcessing/MotionBlur.cpp"
	"PostProcessing/PostProcessing.cpp"
	"Resources/MeshResource.cpp"
	"Resources/BatchResource.cpp"
	"Resources/MaterialResource.cpp"
	"Resources/TextureResource.cpp"
	"Resources/SkeletalMeshResource.cpp"
	"Resources/StaticMeshResource.cpp"
	"Resources/TerrainResource.cpp"
	"Components/BillboardRendererComponent.cpp"
	"Components/LightComponent.cpp"
	"Components/MeshRendererComponentBase.cpp"
	"Components/MeshRendererComponent.cpp"
	"Components/SkeletalMeshRendererComponent.cpp"
	"Components/TerrainRendererComponent.cpp"
	"Components/CameraComponent.cpp"
	"Camera/CameraManager.cpp"
	"CommandSignature.cpp"
	"CommandContext.cpp"
	"Geometry/ModelLoader/FbxLoader.cpp"
	"Geometry/Mesh.cpp"
	"Geometry/GeometryGenerator.cpp"
	"Renderer.cpp"
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

target_link_libraries("Renderer" PRIVATE ${Boost_LIBRARIES} ${FBX_LINK_LIBS} DirectXTex IMGUI PUBLIC ResourceManager Scene PIX::WinPixEventRuntime)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/RendererTests.cpp" INCLUDE "." LINK Renderer PREFIX Renderer)
endif(BUILD_TESTS)