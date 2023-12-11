set(GRAPHICS_LIBS_SOURCES)
set(GRAPHICS_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND GRAPHICS_LIBS_INCLUDE
	"pch.hpp"
	"AmbientOcclusion/ScreenSpaceAmbientOcclusion.hpp"
	"AntiAliasing/TemporalEffect.hpp"
	"CommandContext.hpp"
	"CommandSignature.hpp"
	"d3dx12.h"
	"DeviceResources.hpp"
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
	"GraphicsUtils/Shaders.hpp"
	"GraphicsUtils/StateObject.hpp"
	"PostProcessing/MotionBlur.hpp"
	"PostProcessing/PostProcessing.hpp"
	)

list(APPEND GRAPHICS_LIBS_SOURCES
	"pch.cpp"
	"AmbientOcclusion/ScreenSpaceAmbientOcclusion.cpp"
	"AntiAliasing/TemporalEffect.cpp"
	"CommandContext.cpp"
	"CommandSignature.cpp"
	"DeviceResources.cpp"
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
	"GraphicsUtils/Shaders.cpp"
	"GraphicsUtils/StateObject.cpp"
	"PostProcessing/MotionBlur.cpp"
	"PostProcessing/PostProcessing.cpp"
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

include("${ENGINE_ROOT}/cmake/FindWinPixEventRuntime.cmake")

target_include_directories("Graphics"
PUBLIC
	${WinPixEventRuntime_INCLUDE_DIR}
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
	"${ENGINE_DIR}/Libs/DirectXShaderCompiler/inc"
)

target_link_libraries("Graphics" PRIVATE "${ENGINE_DIR}/Libs/DirectXShaderCompiler/lib/dxcompiler.lib")

target_link_libraries("Graphics" PRIVATE ${Boost_LIBRARIES} DirectXTex IMGUI PUBLIC ResourceManager PIX::WinPixEventRuntime)

if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/GraphicsTests.cpp" INCLUDE "." LINK Graphics PREFIX Graphics)
endif(BUILD_TESTS)