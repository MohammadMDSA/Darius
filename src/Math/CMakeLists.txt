set(MATH_LIBS_SOURCES)
set(MATH_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND MATH_LIBS_INCLUDE
	"Bounds/BoundingBox.hpp"
	"Bounds/BoundingPlane.hpp"
	"Bounds/BoundingSphere.hpp"
	"Color.hpp"
	"Common.hpp"
	"Functions.inl"
	"Matrix3.hpp"
	"Matrix4.hpp"
	"pch.hpp"
	"Plane.hpp"
	"Quaternion.hpp"
	"Rectangle.hpp"
	"Scalar.hpp"
	"Transform.hpp"
	"Vector.hpp"
	"VectorMath.hpp"
	"Viewport.hpp"
	"Camera/Camera.hpp"
	"Camera/Frustum.hpp"
	)

list(APPEND MATH_LIBS_SOURCES
	"Bounds/BoundingSphere.cpp"
	"pch.cpp"
	"Color.cpp"
	"Camera/Camera.cpp"
	"Camera/Frustum.cpp"
	)


add_library("Math")

target_sources("Math"
	PRIVATE
	${MATH_LIBS_SOURCES}
	${MATH_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)


set_target_properties("Math" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Math" PRIVATE pch.hpp )
endif()


target_include_directories("Math"
PRIVATE
	".."
)

target_link_libraries("Math" PRIVATE ${Boost_LIBRARIES})
# target_link_libraries("Math" PRIVATE DirectXTK12)
target_link_libraries("Math" PRIVATE Utils)


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	# add_boost_test(SOURCE "Tests/MathTests.cpp" INCLUDE "." LINK Math PREFIX Math)
endif(BUILD_TESTS)