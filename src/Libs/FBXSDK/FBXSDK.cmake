
set(FBX_INCLUDE_DIRS ${SOURCE_DIR}/Libs/FBXSDK/include)

if(${CMAKE_BUILD_TYPE} MATCHES Release)
	set(FBX_LIB_DIR "${SOURCE_DIR}/Libs/FBXSDK/libs/release")
else()
	set(FBX_LIB_DIR "${SOURCE_DIR}/Libs/FBXSDK/libs/debug")
endif()
set(FBX_LIBS)
list(APPEND FBX_LIBS
	"${FBX_LIB_DIR}/libfbxsdk.lib"
	"${FBX_LIB_DIR}/libxml2.lib"
	"${FBX_LIB_DIR}/zlib.lib"
	)