set(RENDERER_LIBS_SOURCES)
set(RENDERER_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND RENDERER_LIBS_INCLUDE
	"pch.hpp"
	"GraphicsUtils/D3DUtils.hpp"
	"GraphicsUtils/VertexTypes.hpp"
	"GraphicsUtils/GpuResource.hpp"
	"GraphicsUtils/CommandListManager.hpp"
	"GraphicsUtils/CommandAllocatorPool.hpp"
	"GraphicsUtils/RootSignature.hpp"
	"GraphicsUtils/PipelineState.hpp"
	"GraphicsUtils/SamplerManager.hpp"
	"GraphicsUtils/Memory/DescriptorHeap.hpp"
	"GraphicsUtils/Memory/LinearAllocator.hpp"
	"GraphicsUtils/Memory/DynamicDescriptorHeap.hpp"
	"GraphicsUtils/Buffers/PixelBuffer.hpp"
	"GraphicsUtils/Buffers/ColorBuffer.hpp"
	"GraphicsUtils/Buffers/GpuBuffer.hpp"
	"GraphicsUtils/Buffers/UploadBuffer.hpp"
	"GraphicsUtils/Buffers/DepthBuffer.hpp"
	"GraphicsUtils/Buffers/ReadbackBuffer.hpp"
	"Camera/CameraManager.hpp"
	"CommandSignature.hpp"
	"CommandContext.hpp"
	"Geometry/Mesh.hpp"
	"Geometry/MeshData.hpp"
	"Geometry/GeometryGenerator.hpp"
	"Renderer.hpp"
	"GraphicsCore.hpp"
	"DeviceResources.hpp"
	"FrameResource.hpp"
	"LightManager.hpp"
	"d3dx12.h"
	)

list(APPEND RENDERER_LIBS_SOURCES
	"pch.cpp"
	"GraphicsUtils/D3DUtils.cpp"
	"GraphicsUtils/VertexTypes.cpp"
	"GraphicsUtils/GpuResource.cpp"
	"GraphicsUtils/CommandListManager.cpp"
	"GraphicsUtils/CommandAllocatorPool.cpp"
	"GraphicsUtils/RootSignature.cpp"
	"GraphicsUtils/PipelineState.cpp"
	"GraphicsUtils/SamplerManager.cpp"
	"GraphicsUtils/Memory/DescriptorHeap.cpp"
	"GraphicsUtils/Memory/LinearAllocator.cpp"
	"GraphicsUtils/Memory/DynamicDescriptorHeap.cpp"
	"GraphicsUtils/Buffers/PixelBuffer.cpp"
	"GraphicsUtils/Buffers/ColorBuffer.cpp"
	"GraphicsUtils/Buffers/GpuBuffer.cpp"
	"GraphicsUtils/Buffers/UploadBuffer.cpp"
	"GraphicsUtils/Buffers/DepthBuffer.cpp"
	"GraphicsUtils/Buffers/ReadbackBuffer.cpp"
	"Camera/CameraManager.cpp"
	"CommandSignature.cpp"
	"CommandContext.cpp"
	"Geometry/Mesh.cpp"
	"Geometry/GeometryGenerator.cpp"
	"Renderer.cpp"
	"GraphicsCore.cpp"
	"DeviceResources.cpp"
	"FrameResource.cpp"
	"LightManager.cpp"
	)


add_library("Renderer")

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
)

target_link_libraries("Renderer" PRIVATE ${Boost_LIBRARIES} PUBLIC IMGUI)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/RendererTests.cpp" INCLUDE "." LINK Renderer PREFIX Renderer)
endif(BUILD_TESTS)