//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author(s):	James Stanard
//

#include "Common.hlsli"

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TexCoord0;
};

Texture2D<float4> baseColorTexture          : register(t0);
SamplerState baseColorSampler               : register(s0);

cbuffer cbMaterial : register(b0)
{
    float4 gDiffuseAlbedo;
    float3 gFresnelR0;
    float3 gEmissive;
    float1 gMetallic;
    float1 gRoughness;
    uint gTexStats;
}

[RootSignature(Renderer_RootSig)]
void main(VSOutput vsOutput)
{
    float cutoff = f16tof32(gTexStats >> 16);
    if (baseColorTexture.Sample(baseColorSampler, vsOutput.uv).a < cutoff)
        discard;
}
