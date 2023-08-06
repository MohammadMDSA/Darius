#pragma once

#ifdef HLSL
#include "../../Utils/HlslCompat.h"
#else
using namespace DirectX;

typedef UINT16 Index;
#endif

struct SceneConstantBuffer
{
    XMMATRIX gProjectionToWorld;
    XMVECTOR gCameraPosition;
    XMVECTOR gLightAmbientColor;
};

struct Payload
{
    XMFLOAT3 color;
};

#ifdef HLSL
typedef uint NormalDepthTexFormat;
#else
#define COMPACT_NORMAL_DEPTH_DXGI_FORMAT DXGI_FORMAT_R32_UINT
#endif