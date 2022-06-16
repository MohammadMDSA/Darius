set(CORE_LIBS_SOURCES)
set(CORE_LIBS_INCLUDE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ".")

list(APPEND CORE_LIBS_INCLUDE
    "Signal.hpp"
	)

list(APPEND CORE_LIBS_SOURCES
	"Signal.cpp"
	)


add_library("Core")

target_sources("Core"
	PRIVATE 
	${CORE_LIBS_SOURCES})

find_package(Boost REQUIRED)

set_target_properties("Core" PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories("Core" PRIVATE
	${CORE_LIBS_INCLUDE}
	${Boost_INCLUDE_DIRS}
)