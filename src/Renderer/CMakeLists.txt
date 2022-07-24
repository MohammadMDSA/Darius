set(RENDERER_LIBS_SOURCES)
set(RENDERER_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND RENDERER_LIBS_INCLUDE
	"pch.hpp"
	"GraphicsUtils/D3DUtils.hpp"
	"GraphicsUtils/UploadBuffer.hpp"
	"GraphicsUtils/VertexTypes.hpp"
	"GraphicsUtils/DescriptorHeap.hpp"
	"Camera/CameraManager.hpp"
	"Geometry/Mesh.hpp"
	"Geometry/MeshData.hpp"
	"Geometry/GeometryGenerator.hpp"
	"Renderer.hpp"
	"DeviceResources.hpp"
	"FrameResource.hpp"
	"d3dx12.h"
	)

list(APPEND RENDERER_LIBS_SOURCES
	"pch.cpp"
	"GraphicsUtils/D3DUtils.cpp"
	"GraphicsUtils/VertexTypes.cpp"
	"GraphicsUtils/DescriptorHeap.cpp"
	"Camera/CameraManager.cpp"
	"Geometry/Mesh.cpp"
	"Geometry/GeometryGenerator.cpp"
	"Renderer.cpp"
	"DeviceResources.cpp"
	"FrameResource.cpp"
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