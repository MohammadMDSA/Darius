
set(FBX_INCLUDE_DIRS ${SOURCE_DIR}/Libs/FBXSDK/include)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
	set(FBX_LIB_DIR "${SOURCE_DIR}/Libs/FBXSDK/libs/debug")
else()
	set(FBX_LIB_DIR "${SOURCE_DIR}/Libs/FBXSDK/libs/release")
endif()
set(FBX_LIBS)
list(APPEND FBX_LIBS
	"${FBX_LIB_DIR}/libfbxsdk.lib"
	"${FBX_LIB_DIR}/libxml2-mt.lib"
	"${FBX_LIB_DIR}/zlib-mt.lib"
	)