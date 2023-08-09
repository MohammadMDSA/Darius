#ifndef __RT_MATERIAL_HLSLI__
#define __RT_MATERIAL_HLSLI__

#include "../../Utils/MaterialCommon.hlsli"

struct PrimitiveMaterialBuffer
{
    XMFLOAT3                Albedo;
    XMFLOAT3                Specular;
    float                   SpecularMask;
    XMFLOAT3                Metallic;
    XMFLOAT3                Transmissivity;
    XMFLOAT3                Emission;
    XMFLOAT3                Opacity;
    XMFLOAT3                Eta;
    float                   Roughness;
    MaterialType::Type      Type;
};

#endif