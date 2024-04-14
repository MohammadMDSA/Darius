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

#ifndef __TONE_MAPPING_UTILITY_HLSLI__
#define __TONE_MAPPING_UTILITY_HLSLI__

#include "../Utils/Common.hlsli"
#include "../Utils/ShaderUtility.hlsli"
#include "../Utils/ACES.hlsli"

//
// Reinhard
// 

// The Reinhard tone operator.  Typically, the value of k is 1.0, but you can adjust exposure by 1/k.
// I.e. TM_Reinhard(x, 0.5) == TM_Reinhard(x * 2.0, 1.0)
float3 TM_Reinhard(float3 hdr, float k = 1.0)
{
    return hdr / (hdr + k);
}

// The inverse of Reinhard
float3 ITM_Reinhard(float3 sdr, float k = 1.0)
{
    return k * sdr / (k - sdr);
}

//
// Reinhard-Squared
//

// This has some nice properties that improve on basic Reinhard.  Firstly, it has a "toe"--that nice,
// parabolic upswing that enhances contrast and color saturation in darks.  Secondly, it has a long
// shoulder giving greater detail in highlights and taking longer to desaturate.  It's invertible, scales
// to HDR displays, and is easy to control.
//
// The default constant of 0.25 was chosen for two reasons.  It maps closely to the effect of Reinhard
// with a constant of 1.0.  And with a constant of 0.25, there is an inflection point at 0.25 where the
// curve touches the line y=x and then begins the shoulder.
//
// Note:  If you are currently using ACES and you pre-scale by 0.6, then k=0.30 looks nice as an alternative
// without any other adjustments.

float3 TM_ReinhardSq(float3 hdr, float k = 0.25)
{
    float3 reinhard = hdr / (hdr + k);
    return reinhard * reinhard;
}

float3 ITM_ReinhardSq(float3 sdr, float k = 0.25)
{
    return k * (sdr + sqrt(sdr)) / (1.0 - sdr);
}

//
// Stanard (New)
//

// This is the new tone operator.  It resembles ACES in many ways, but it is simpler to evaluate with ALU.  One
// advantage it has over Reinhard-Squared is that the shoulder goes to white more quickly and gives more overall
// brightness and contrast to the image.

float3 TM_Stanard(float3 hdr)
{
    return TM_Reinhard(hdr * sqrt(hdr), sqrt(4.0 / 27.0));
}

float3 ITM_Stanard(float3 sdr)
{
    return pow(ITM_Reinhard(sdr, sqrt(4.0 / 27.0)), 2.0 / 3.0);
}

//
// Stanard (Old)
//

// This is the old tone operator first used in HemiEngine and then MiniEngine.  It's simplistic, efficient,
// invertible, and gives nice results, but it has no toe, and the shoulder goes to white fairly quickly.
//
// Note that I removed the distinction between tone mapping RGB and tone mapping Luma.  Philosophically, I
// agree with the idea of trying to remap brightness to displayable values while preserving hue.  But you
// run into problems where one or more color channels end up brighter than 1.0 and get clipped.

float3 ToneMap(float3 hdr)
{
    return 1 - exp2(-hdr);
}

float3 InverseToneMap(float3 sdr)
{
    return -log2(max(1e-6, 1 - sdr));
}

float ToneMapLuma(float luma)
{
    return 1 - exp2(-luma);
}

float InverseToneMapLuma(float luma)
{
    return -log2(max(1e-6, 1 - luma));
}

//
// ACES
//

// The next generation of filmic tone operators.

float3 ToneMapACES(float3 hdr)
{
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return saturate((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E));
}

float3 InverseToneMapACES(float3 sdr)
{
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return 0.5 * (D * sdr - sqrt(((D * D - 4 * C * E) * sdr + 4 * A * E - 2 * B * D) * sdr + B * B) - B) / (A - C * sdr);
}

// Copyright Epic Games, Inc. All Rights Reserved.

// Unreal Engine Film ToneMapping
float3 FilmToneMap(float3 LinearColor, float FilmSlope, float FilmToe, float FilmShoulder, float FilmBlackClip, float FilmWhiteClip)
{
    const float3x3 sRGB_2_AP0 = mul(XYZ_2_AP0_MAT, mul(D65_2_D60_CAT, sRGB_2_XYZ_MAT));
    const float3x3 sRGB_2_AP1 = mul(XYZ_2_AP1_MAT, mul(D65_2_D60_CAT, sRGB_2_XYZ_MAT));

    const float3x3 AP0_2_sRGB = mul(XYZ_2_sRGB_MAT, mul(D60_2_D65_CAT, AP0_2_XYZ_MAT));
    const float3x3 AP1_2_sRGB = mul(XYZ_2_sRGB_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));
	
    const float3x3 AP0_2_AP1 = mul(XYZ_2_AP1_MAT, AP0_2_XYZ_MAT);
    const float3x3 AP1_2_AP0 = mul(XYZ_2_AP0_MAT, AP1_2_XYZ_MAT);
	
    float3 ColorAP1 = LinearColor;
	//float3 ColorAP1 = mul( sRGB_2_AP1, float3(LinearColor) );

    float3 ColorAP0 = mul(AP1_2_AP0, ColorAP1);

	// "Glow" module constants
    const float RRT_GLOW_GAIN = 0.05;
    const float RRT_GLOW_MID = 0.08;

    float saturation = rgb_2_saturation(ColorAP0);
    float ycIn = rgb_2_yc(ColorAP0);
    float s = sigmoid_shaper((saturation - 0.4) / 0.2);
    float addedGlow = 1 + glow_fwd(ycIn, RRT_GLOW_GAIN * s, RRT_GLOW_MID);
    ColorAP0 *= addedGlow;

	// --- Red modifier --- //
    const float RRT_RED_SCALE = 0.82;
    const float RRT_RED_PIVOT = 0.03;
    const float RRT_RED_HUE = 0;
    const float RRT_RED_WIDTH = 135;
    float hue = rgb_2_hue(ColorAP0);
    float centeredHue = center_hue(hue, RRT_RED_HUE);
    float hueWeight = Square(smoothstep(0, 1, 1 - abs(2 * centeredHue / RRT_RED_WIDTH)));
		
    ColorAP0.r += hueWeight * saturation * (RRT_RED_PIVOT - ColorAP0.r) * (1. - RRT_RED_SCALE);
	
	// Use ACEScg primaries as working space
    float3 WorkingColor = mul(AP0_2_AP1_MAT, ColorAP0);

    WorkingColor = max(0, WorkingColor);

	// Pre desaturate
    WorkingColor = lerp(dot(WorkingColor, AP1_RGB2Y), WorkingColor, 0.96);
	
    const float ToeScale = 1 + FilmBlackClip - FilmToe;
    const float ShoulderScale = 1 + FilmWhiteClip - FilmShoulder;
	
    const float InMatch = 0.18;
    const float OutMatch = 0.18;

    float ToeMatch;
    if (FilmToe > 0.8)
    {
		// 0.18 will be on straight segment
        ToeMatch = (1 - FilmToe - OutMatch) / FilmSlope + log10(InMatch);
    }
    else
    {
		// 0.18 will be on toe segment

		// Solve for ToeMatch such that input of InMatch gives output of OutMatch.
        const float bt = (OutMatch + FilmBlackClip) / ToeScale - 1;
        ToeMatch = log10(InMatch) - 0.5 * log((1 + bt) / (1 - bt)) * (ToeScale / FilmSlope);
    }

    float StraightMatch = (1 - FilmToe) / FilmSlope - ToeMatch;
    float ShoulderMatch = FilmShoulder / FilmSlope - StraightMatch;
	
    float3 LogColor = log10(WorkingColor);
    float3 StraightColor = FilmSlope * (LogColor + StraightMatch);
	
    float3 ToeColor = (-FilmBlackClip) + (2 * ToeScale) / (1 + exp((-2 * FilmSlope / ToeScale) * (LogColor - ToeMatch)));
    float3 ShoulderColor = (1 + FilmWhiteClip) - (2 * ShoulderScale) / (1 + exp((2 * FilmSlope / ShoulderScale) * (LogColor - ShoulderMatch)));
    
    ToeColor = select(LogColor < ToeMatch, ToeColor, StraightColor);
    ShoulderColor = select(LogColor > ShoulderMatch, ShoulderColor, StraightColor);

    float3 t = saturate((LogColor - ToeMatch) / (ShoulderMatch - ToeMatch));
    t = ShoulderMatch < ToeMatch ? 1 - t : t;
    t = (3 - 2 * t) * t * t;
    float3 ToneColor = lerp(ToeColor, ShoulderColor, t);

	// Post desaturate
    ToneColor = lerp(dot(float3(ToneColor), AP1_RGB2Y), ToneColor, 0.93);

	// Returning positive AP1 values
    return max(0, ToneColor);
}

float3 FilmToneMapInverse(float3 ToneColor)
{
    const float3x3 sRGB_2_AP1 = mul(XYZ_2_AP1_MAT, mul(D65_2_D60_CAT, sRGB_2_XYZ_MAT));
    const float3x3 AP1_2_sRGB = mul(XYZ_2_sRGB_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));
	
	// Use ACEScg primaries as working space
    float3 WorkingColor = mul(sRGB_2_AP1, saturate(ToneColor));

    WorkingColor = max(0, WorkingColor);
	
	// Post desaturate
    WorkingColor = lerp(dot(WorkingColor, AP1_RGB2Y), WorkingColor, 1.0 / 0.93);

    float3 ToeColor = 0.374816 * pow(0.9 / min(WorkingColor, 0.8) - 1, -0.588729);
    float3 ShoulderColor = 0.227986 * pow(1.56 / (1.04 - WorkingColor) - 1, 1.02046);

    float3 t = saturate((WorkingColor - 0.35) / (0.45 - 0.35));
    t = (3 - 2 * t) * t * t;
    float3 LinearColor = lerp(ToeColor, ShoulderColor, t);

	// Pre desaturate
    LinearColor = lerp(dot(LinearColor, AP1_RGB2Y), LinearColor, 1.0 / 0.96);

    LinearColor = mul(AP1_2_sRGB, LinearColor);

	// Returning positive sRGB values
    return max(0, LinearColor);
}

#endif // __TONE_MAPPING_UTILITY_HLSLI__
