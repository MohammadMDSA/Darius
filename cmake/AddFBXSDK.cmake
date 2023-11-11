if(NOT DEFINED FBX_ROOT)
	SET(FBX_ROOT $ENV{FBXSDK_ROOT})
endif(NOT DEFINED FBX_ROOT)

set(FBX_INCLUDE_DIRS
	"${FBX_ROOT}/include")

if(EXISTS "${FBX_ROOT}/lib/vs2022/x64/release/libfbxsdk.lib")
	set(FBX_LINK_LIBS "${FBX_ROOT}/lib/vs2022/x64/release/libfbxsdk.lib")
elseif(EXISTS "${FBX_ROOT}/lib/vs2019/x64/release/libfbxsdk.lib")
	set(FBX_LINK_LIBS "${FBX_ROOT}/lib/vs2019/x64/release/libfbxsdk.lib")
else()
	message(FATAL_ERROR "Proper build of libfbxsdk.lib was not found under ${FBX_ROOT}/lib/")
endif()