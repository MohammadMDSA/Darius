set(CORE_LIBS_SOURCES)
set(CORE_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND CORE_LIBS_INCLUDE
    "Signal.hpp"
	"Memory.hpp"
	)

list(APPEND CORE_LIBS_SOURCES
	"Signal.cpp"
	"Memory.cpp"
	)


add_library("Core")

target_sources("Core"
	PRIVATE 
	${CORE_LIBS_SOURCES})

	
set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_ARCHITECTURE "-x64")
find_package(Boost REQUIRED)

set_target_properties("Core" PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories("Core"
PUBLIC
	${CORE_LIBS_INCLUDE}
PRIVATE
	${Boost_INCLUDE_DIRS}
)

target_link_libraries("Core" PRIVATE ${Boost_LIBRARIES})