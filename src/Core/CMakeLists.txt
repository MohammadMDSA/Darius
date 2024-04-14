set(CORE_LIBS_SOURCES)
set(CORE_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND CORE_LIBS_INCLUDE
	"Containers/ConcurrentQueue.hpp"
	"Containers/EnumAsByte.hpp"
	"Containers/Vector.hpp"
	"Containers/Set.hpp"
	"Containers/List.hpp"
	"Serialization/Json.hpp"
	"Serialization/TypeSerializer.hpp"
	"Exceptions/Exception.hpp"
	"TimeManager/StepTimer.hpp"
	"TimeManager/TimeManager.hpp"
	"TimeManager/SystemTime.hpp"
	"Memory/Memory.hpp"
	"Memory/Allocators/PagedAllocator.hpp"
	"MultiThreading/SafeNumeric.hpp"
	"MultiThreading/SpinLock.hpp"
	"Filesystem/FileUtils.hpp"
	"Filesystem/Path.hpp"
    "Signal.hpp"
	"Input.hpp"
	"RefCounting/Counted.hpp"
	"RefCounting/Ref.hpp"
	"Hash.hpp"
	"Uuid.hpp"
	"pch.hpp"
	)

list(APPEND CORE_LIBS_SOURCES
	"TimeManager/TimeManager.cpp"
	"TimeManager/SystemTime.cpp"
	"Containers/Containers.cpp"
	"Serialization/Json.cpp"
	"Serialization/TypeSerializer.cpp"
	"Filesystem/FileUtils.cpp"
	"Memory/Memory.cpp"
	"RefCounting/Counted.cpp"
	"Signal.cpp"
	"Input.cpp"
	"Uuid.cpp"
	)


add_library("Core")

target_sources("Core"
	PRIVATE
	${CORE_LIBS_SOURCES}
	${CORE_LIBS_INCLUDE})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(BOOST 1.75 REQUIRED)

set_target_properties("Core" PROPERTIES LINKER_LANGUAGE CXX)

if ((${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16") AND (NOT MINGW))
    target_precompile_headers("Core" PRIVATE pch.hpp )
endif()


include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

target_include_directories("Core"
PUBLIC
	"${SOURCE_DIR}/Generated"
SYSTEM
	"${SOURCE_DIR}/Libs/rttr/src"
PRIVATE
	".."
	${Boost_INCLUDE_DIRS}
)

target_link_libraries("Core" PUBLIC Utils rttr_core PRIVATE ${Boost_LIBRARIES} DirectXTK12)

# Adding Nlohmann Json
target_include_directories("Core" PUBLIC "${SOURCE_DIR}/Libs/nlohmann_json/single_include")


if(BUILD_TESTS)
	set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
	include(BoostTestHelper)
	add_compile_definitions(BOOST_TEST_LOG_LEVEL=all)
	add_compile_definitions(BOOST_TEST_DETECT_MEMORY_LEAK=1)
	add_compile_definitions(BOOST_TEST_SHOW_PROGRESS=yes)
	add_boost_test(SOURCE "Tests/CoreTests.cpp" INCLUDE "." LINK Core PREFIX Core)
endif(BUILD_TESTS)