if(NOT DEFINED FBX_ROOT)
	SET(FBX_ROOT $ENV{FBXSDK_ROOT})
endif(NOT DEFINED FBX_ROOT)

set(FBX_INCLUDE_DIRS
	"${FBX_ROOT}/include")

set(FBX_LINK_LIBS
	"${FBX_ROOT}/lib/vs2019/x64/release/libfbxsdk.lib")
