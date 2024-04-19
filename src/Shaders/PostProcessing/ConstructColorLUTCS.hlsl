// Copyright Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	LUTBlender.usf: Filter pixel shader source.   
=============================================================================*/

#include "../Utils/ACES.hlsli"
#include "../Utils/ColorSpaceUtility.hlsli"
#include "ToneMapCommon.hlsli"

#define USE_VOLUME_LUT 1

cbuffer Constants : register(b1)
{
    float3 ColorScale;
    float4 OverlayColor;
    float3 InverseGamma;
    float2 RcpBufferDim;
    
    uint IsTemperatureWhiteBalance;

    float LUTSize;

    float WhiteTemp;
    float WhiteTint;
    uint OutputGamut;
    
    float4 ColorSaturation;
    float4 ColorContrast;
    float4 ColorGamma;
    float4 ColorGain;
    float4 ColorOffset;

    float4 ColorSaturationShadows;
    float4 ColorContrastShadows;
    float4 ColorGammaShadows;
    float4 ColorGainShadows;
    float4 ColorOffsetShadows;

    float4 ColorSaturationMidtones;
    float4 ColorContrastMidtones;
    float4 ColorGammaMidtones;
    float4 ColorGainMidtones;
    float4 ColorOffsetMidtones;

    float4 ColorSaturationHighlights;
    float4 ColorContrastHighlights;
    float4 ColorGammaHighlights;
    float4 ColorGainHighlights;
    float4 ColorOffsetHighlights;

    float ColorCorrectionShadowsMax;
    float ColorCorrectionHighlightsMin;
    float ColorCorrectionHighlightsMax;
    
    float BlueCorrection;
    float ExpandGamut;
    float ToneCurveAmount;
    
    float4 ACESMinMaxData;
    float4 ACESMidData;
    float4 ACESCoefsLow_0;
    float4 ACESCoefsHigh_0;
    float ACESCoefsLow_4;
    float ACESCoefsHigh_4;
    float ACESSceneColorMultiplier;

}
RWTexture3D<float4> RWOutputTexture : register(u0);

float3 ColorCorrect(float3 WorkingColor,
	float4 ColorSaturation,
	float4 ColorContrast,
	float4 ColorGamma,
	float4 ColorGain,
	float4 ColorOffset)
{
	// TODO optimize
    float Luma = dot(WorkingColor, AP1_RGB2Y);
    WorkingColor = max(0, lerp(Luma.xxx, WorkingColor, ColorSaturation.xyz * ColorSaturation.w));
    WorkingColor = pow(WorkingColor * (1.0 / 0.18), ColorContrast.xyz * ColorContrast.w) * 0.18;
    WorkingColor = pow(WorkingColor, 1.0 / (ColorGamma.xyz * ColorGamma.w));
    WorkingColor = WorkingColor * (ColorGain.xyz * ColorGain.w) + (ColorOffset.xyz + ColorOffset.w);
    return WorkingColor;
}

// Nuke-style Color Correct
float3 ColorCorrectAll(float3 WorkingColor)
{
    float Luma = dot(WorkingColor, AP1_RGB2Y);

	// Shadow CC
    float3 CCColorShadows = ColorCorrect(WorkingColor,
		ColorSaturationShadows * ColorSaturation,
		ColorContrastShadows * ColorContrast,
		ColorGammaShadows * ColorGamma,
		ColorGainShadows * ColorGain,
		ColorOffsetShadows + ColorOffset);
    float CCWeightShadows = 1 - smoothstep(0, ColorCorrectionShadowsMax, Luma);
	
	// Highlight CC
    float3 CCColorHighlights = ColorCorrect(WorkingColor,
		ColorSaturationHighlights * ColorSaturation,
		ColorContrastHighlights * ColorContrast,
		ColorGammaHighlights * ColorGamma,
		ColorGainHighlights * ColorGain,
		ColorOffsetHighlights + ColorOffset);
    float CCWeightHighlights = smoothstep(ColorCorrectionHighlightsMin, ColorCorrectionHighlightsMax, Luma);

	// Midtone CC
    float3 CCColorMidtones = ColorCorrect(WorkingColor,
		ColorSaturationMidtones * ColorSaturation,
		ColorContrastMidtones * ColorContrast,
		ColorGammaMidtones * ColorGamma,
		ColorGainMidtones * ColorGain,
		ColorOffsetMidtones + ColorOffset);
    float CCWeightMidtones = 1 - CCWeightShadows - CCWeightHighlights;

	// Blend Shadow, Midtone and Highlight CCs
    float3 WorkingColorSMH = CCColorShadows * CCWeightShadows + CCColorMidtones * CCWeightMidtones + CCColorHighlights * CCWeightHighlights;
	
    return WorkingColorSMH;
}

uint GetOutputDevice()
{
	return TONEMAPPER_OUTPUT_sRGB;
}

float4 CombineLUTsCommon(float2 InUV, uint InLayerIndex)
{
#if USE_VOLUME_LUT == 1
	// construct the neutral color from a 3d position volume texture	
	float4 Neutral;
	{
		float2 UV = InUV - float2(0.5f / LUTSize, 0.5f / LUTSize);

		Neutral = float4(UV * LUTSize / (LUTSize - 1), InLayerIndex / (LUTSize - 1), 0);
	}
#else
	// construct the neutral color from a 2d position in 256x16
    float4 Neutral;
	{ 
        float2 UV = InUV;

		// 0.49999f instead of 0.5f to avoid getting into negative values
        UV -= float2(0.49999f / (LUTSize * LUTSize), 0.49999f / LUTSize);

        float Scale = LUTSize / (LUTSize - 1);

        float3 RGB;
		
        RGB.r = frac(UV.x * LUTSize);
        RGB.b = UV.x - RGB.r / LUTSize;
        RGB.g = UV.y;

        Neutral = float4(RGB * Scale, 0);
    }
#endif

    float4 OutColor = 0;
	
    const float3x3 AP1_2_sRGB = mul(XYZ_2_sRGB_MAT, mul(D60_2_D65_CAT, AP1_2_XYZ_MAT));
    const float3x3 sRGB_2_AP1 = mul(XYZ_2_AP1_MAT, mul(D65_2_D60_CAT, sRGB_2_XYZ_MAT));
    const float3x3 AP0_2_AP1 = mul(XYZ_2_AP1_MAT, AP0_2_XYZ_MAT);
    const float3x3 AP1_2_AP0 = mul(XYZ_2_AP0_MAT, AP1_2_XYZ_MAT);

    const float3x3 AP1_2_Output = OuputGamutMappingMatrix(OutputGamut);

    float3 LUTEncodedColor = Neutral.rgb;
    float3 LinearColor;
	// Decode texture values as ST-2084 (Dolby PQ)
    if (GetOutputDevice() >= TONEMAPPER_OUTPUT_ACES1000nitST2084)
    {
		// Since ST2084 returns linear values in nits, divide by a scale factor to convert
		// the reference nit result to be 1.0 in linear.
		// (for efficiency multiply by precomputed inverse)
        LinearColor = ST2084ToLinear(LUTEncodedColor) * LinearToNitsScaleInverse;
    }
	// Decode log values
    else
    {
        LinearColor = LogToLin(LUTEncodedColor) - LogToLin(0);
    }
    
#if !SKIP_TEMPERATURE
    float3 BalancedColor = WhiteBalance(LinearColor, WhiteTemp, WhiteTint, IsTemperatureWhiteBalance, (float3x3) WorkingColorSpace.ToXYZ, (float3x3) WorkingColorSpace.FromXYZ);
#else
	float3 BalancedColor = LinearColor;
#endif

    float3 ColorAP1 = mul(sRGB_2_AP1, BalancedColor);

	// Expand bright saturated colors outside the sRGB gamut to fake wide gamut rendering.
    float LumaAP1 = dot(ColorAP1, AP1_RGB2Y);
    float3 ChromaAP1 = ColorAP1 / LumaAP1;

    float ChromaDistSqr = dot(ChromaAP1 - 1, ChromaAP1 - 1);
    float ExpandAmount = (1 - exp2(-4 * ChromaDistSqr)) * (1 - exp2(-4 * ExpandGamut * LumaAP1 * LumaAP1));

	// Bizarre matrix but this expands sRGB to between P3 and AP1
	// CIE 1931 chromaticities:	x		y
	//				Red:		0.6965	0.3065
	//				Green:		0.245	0.718
	//				Blue:		0.1302	0.0456
	//				White:		0.3127	0.329
    const float3x3 Wide_2_XYZ_MAT =
    {
        0.5441691, 0.2395926, 0.1666943,
		0.2394656, 0.7021530, 0.0583814,
		-0.0023439, 0.0361834, 1.0552183,
    };

    const float3x3 Wide_2_AP1 = mul(XYZ_2_AP1_MAT, Wide_2_XYZ_MAT);
    const float3x3 ExpandMat = mul(Wide_2_AP1, AP1_2_sRGB);

    float3 ColorExpand = mul(ExpandMat, ColorAP1);
    ColorAP1 = lerp(ColorAP1, ColorExpand, ExpandAmount);

    ColorAP1 = ColorCorrectAll(ColorAP1);

	// Store for Linear HDR output without tone curve
    float3 GradedColor = mul(AP1_2_sRGB, ColorAP1);

    const float3x3 BlueCorrect =
    {
        0.9404372683, -0.0183068787, 0.0778696104,
		0.0083786969, 0.8286599939, 0.1629613092,
		0.0005471261, -0.0008833746, 1.0003362486
    };
    const float3x3 BlueCorrectInv =
    {
        1.06318, 0.0233956, -0.0865726,
		-0.0106337, 1.20632, -0.19569,
		-0.000590887, 0.00105248, 0.999538
    };
    const float3x3 BlueCorrectAP1 = mul(AP0_2_AP1, mul(BlueCorrect, AP1_2_AP0));
    const float3x3 BlueCorrectInvAP1 = mul(AP0_2_AP1, mul(BlueCorrectInv, AP1_2_AP0));

	// Blue correction
    ColorAP1 = lerp(ColorAP1, mul(BlueCorrectAP1, ColorAP1), BlueCorrection);

	// Tonemapped color in the AP1 gamut
    float3 ToneMappedColorAP1 = FilmToneMap(ColorAP1);
    ColorAP1 = lerp(ColorAP1, ToneMappedColorAP1, ToneCurveAmount);

	// Uncorrect blue to maintain white point
    ColorAP1 = lerp(ColorAP1, mul(BlueCorrectInvAP1, ColorAP1), BlueCorrection);

	// Convert from AP1 to the working color space and clip out-of-gamut values
    float3 FilmColor = max(0, mul(AP1_2_sRGB, ColorAP1));
    
	// apply math color correction on top to texture based solution
    //FilmColor = ColorCorrection(FilmColor);

	// blend with custom LDR color, used for Fade track in Cinematics
    float3 FilmColorNoGamma = lerp(FilmColor * ColorScale, OverlayColor.rgb, OverlayColor.a);
	// Apply Fade track to linear outputs also
    GradedColor = lerp(GradedColor * ColorScale, OverlayColor.rgb, OverlayColor.a);


	// Apply "gamma" curve adjustment.
    FilmColor = pow(max(0, FilmColorNoGamma), InverseGamma.y);
		
	// Note: Any changes to the following device encoding logic must also be made 
	// in PostProcessDeviceEncodingOnly.usf's corresponding pixel shader.
    float3 OutDeviceColor = 0;


	// sRGB, user specified gamut
    //if (GetOutputDevice() == TONEMAPPER_OUTPUT_sRGB)
    {
		// Convert from sRGB to specified output gamut	
		//float3 OutputGamutColor = mul( AP1_2_Output, mul( sRGB_2_AP1, FilmColor ) );

		// FIXME: Workaround for UE-29935, pushing all colors with a 0 component to black output
		// Default parameters seem to cancel out (sRGB->XYZ->AP1->XYZ->sRGB), so should be okay for a temp fix
        float3 OutputGamutColor = FilmColor;

		// Apply conversion to sRGB (this must be an exact sRGB conversion else darks are bad).
        OutDeviceColor = OutputGamutColor;
    }
 

	// Better to saturate(lerp(a,b,t)) than lerp(saturate(a),saturate(b),t)
    OutColor.rgb = OutDeviceColor / 1.05;
    OutColor.a = 0;

    return OutColor;
}

// xy: Unused, zw: ThreadToUVScale
[numthreads(8, 8, 8)]
void main(uint3 DispatchThreadId : SV_DispatchThreadID) 
{
    float2 UV = ((float2) DispatchThreadId.xy + 0.5f) * RcpBufferDim;
	uint LayerIndex = DispatchThreadId.z;
	
	float4 OutColor = CombineLUTsCommon(UV, LayerIndex);

	uint3 PixelPos = DispatchThreadId;
	RWOutputTexture[PixelPos] = OutColor;
}