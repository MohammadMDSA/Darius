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
// Author:  James Stanard 
//

#include "ToneMappingUtility.hlsli"
#include "ToneMapCommon.hlsli"
#include "PostEffectsRS.hlsli"
#include "../Utils/PixelPacking/PixelPacking.hlsli"

StructuredBuffer<float> Exposure : register( t0 );
Texture2D<float3> Bloom     : register( t1 );
Texture3D<float3> LUT       : register( t3 );
#if SUPPORT_TYPED_UAV_LOADS
RWTexture2D<float3> ColorRW : register( u0 );
#else
RWTexture2D<uint> DstColor : register( u0 );
Texture2D<float3> SrcColor : register( t2 );
#endif
RWTexture2D<float> OutLuma : register( u1 );

cbuffer CB0 : register(b1)
{
    float2 g_RcpBufferDim;
    float g_BloomStrength;
    float PaperWhiteRatio; // PaperWhite / MaxBrightness
    float MaxBrightness;
    float LUTSize;
    float InvLUTSize; // 1 / LUTSize
    float LUTScale; // (LUTSize - 1) / LUTSize
    float LUTOffset; // 0.5 / LUTSize
    float Gamma;
    uint ColorGradingEnable;
};

float3 ColorLookupTable(float3 LinearColor)
{
    float3 LUTEncodedColor;

    LUTEncodedColor = LinToLog(LinearColor + LogToLin(0));


    float3 UVW = LUTEncodedColor * LUTScale + LUTOffset;

    float3 OutDeviceColor = LUT.SampleLevel(BilinearSampler, UVW, 0.f).rgb;
    
    return OutDeviceColor * 1.05;
}


[RootSignature(PostEffects_RootSig)]
[numthreads( 8, 8, 1 )]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float2 TexCoord = (DTid.xy + 0.5) * g_RcpBufferDim;

    // Load HDR and bloom
#if SUPPORT_TYPED_UAV_LOADS
    float3 hdrColor = ColorRW[DTid.xy];
#else
    float3 hdrColor = SrcColor[DTid.xy];
#endif

    hdrColor += g_BloomStrength * Bloom.SampleLevel(LinearSampler, TexCoord, 0);
    hdrColor *= Exposure[0];

#if ENABLE_HDR_DISPLAY_MAPPING

    hdrColor = TM_Stanard(REC709toREC2020(hdrColor) * PaperWhiteRatio) * MaxBrightness;
    // Write the HDR color as-is and defer display mapping until we composite with UI
#if SUPPORT_TYPED_UAV_LOADS
    ColorRW[DTid.xy] = hdrColor;
#else
    DstColor[DTid.xy] = Pack_R11G11B10_FLOAT(hdrColor);
#endif
    OutLuma[DTid.xy] = RGBToLogLuminance(hdrColor);

#else

    const float3x3 sRGB_2_AP1 = mul(XYZ_2_AP1_MAT, mul(D65_2_D60_CAT, sRGB_2_XYZ_MAT));
    const float3x3 AP1_2_sRGB = mul(XYZ_2_sRGB_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));
    
    float3 toneMappedAP1 = FilmToneMap(mul(sRGB_2_AP1, hdrColor));
    float3 toneMappedSRGB = mul(AP1_2_sRGB, toneMappedAP1);
    float3 outColor;
    
    // Applying color grading
    if (ColorGradingEnable)
        outColor = mul(AP1_2_sRGB, ColorLookupTable(toneMappedSRGB));
    else
        outColor = toneMappedSRGB;
    
    float3 sdrColor = RemoveSRGBCurve(outColor);
    OutLuma[DTid.xy] = RGBToLogLuminance(sdrColor);

#if SUPPORT_TYPED_UAV_LOADS
    ColorRW[DTid.xy] = sdrColor;
#else
    DstColor[DTid.xy] = Pack_R11G11B10_FLOAT(sdrColor);
#endif


#endif
}
