set(MEMALOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/memory-allocators)

ADD_LIBRARY(MEMALOC STATIC)

target_sources(MEMALOC
    PRIVATE
    ${MEMALOC_DIR}/src/Allocator.cpp
   	${MEMALOC_DIR}/src/CAllocator.cpp
   	${MEMALOC_DIR}/src/LinearAllocator.cpp
   	${MEMALOC_DIR}/src/StackAllocator
   	${MEMALOC_DIR}/src/PoolAllocator
   	${MEMALOC_DIR}/src/FreeListAllocator.cpp
   	${MEMALOC_DIR}/src/Benchmark.cpp 
	${MEMALOC_DIR}/src/main.cpp)

target_include_directories(MEMALOC
    PRIVATE
    ${MEMALOC_DIR}/includes
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    )