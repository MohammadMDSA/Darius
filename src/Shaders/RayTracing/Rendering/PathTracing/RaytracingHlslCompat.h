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
