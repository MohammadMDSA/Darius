#pragma once

#include "Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "ToneMapCommon.hpp"

#include <Math/VectorMath.hpp>

#ifndef D_GRAPHICS_PP
#define D_GRAPHICS_PP Darius::Graphics::PostProcessing
#endif

namespace Darius::Graphics
{
    class ComputeContext;

    namespace Utils
    {
        class RootSignature;
    }
}

namespace Darius::Graphics::PostProcessing
{
	class ComputeColorLUT
	{
        D_STATIC_ASSERT(sizeof(D_MATH::Vector2) == 2 * sizeof(float));
        D_STATIC_ASSERT(sizeof(D_MATH::Vector3) == 4 * sizeof(float));
        D_STATIC_ASSERT(sizeof(D_MATH::Vector4) == 4 * sizeof(float));

    public:
        
		struct ShaderParams
		{
            D_MATH::Vector3     ColorScale;
            D_MATH::Vector4     OverlayColor;
            D_MATH::Vector3     InverseGamma;
            D_MATH::Vector2     InverseBufferSize;

            uint32_t            IsTemperatureWhiteBalance;

            float               LUTSize;

            float               WhiteTemp;
            float               WhiteTint;
            uint32_t            OutputGamut;

            D_MATH::Vector4     ColorSaturation;
            D_MATH::Vector4     ColorContrast;
            D_MATH::Vector4     ColorGamma;
            D_MATH::Vector4     ColorGain;
            D_MATH::Vector4     ColorOffset;

            D_MATH::Vector4     ColorSaturationShadows;
            D_MATH::Vector4     ColorContrastShadows;
            D_MATH::Vector4     ColorGammaShadows;
            D_MATH::Vector4     ColorGainShadows;
            D_MATH::Vector4     ColorOffsetShadows;

            D_MATH::Vector4     ColorSaturationMidtones;
            D_MATH::Vector4     ColorContrastMidtones;
            D_MATH::Vector4     ColorGammaMidtones;
            D_MATH::Vector4     ColorGainMidtones;
            D_MATH::Vector4     ColorOffsetMidtones;

            D_MATH::Vector4     ColorSaturationHighlights;
            D_MATH::Vector4     ColorContrastHighlights;
            D_MATH::Vector4     ColorGammaHighlights;
            D_MATH::Vector4     ColorGainHighlights;
            D_MATH::Vector4     ColorOffsetHighlights;

            float               ColorCorrectionShadowsMax;
            float               ColorCorrectionHighlightsMin;
            float               ColorCorrectionHighlightsMax;

            float               BlueCorrection;
            float               ExpandGamut;
            float               ToneCurveAmount;

            D_MATH::Vector4     ACESMinMaxData;
            D_MATH::Vector4     ACESMidData;
            D_MATH::Vector4     ACESCoefsLow_0;
            D_MATH::Vector4     ACESCoefsHigh_0;
            float               ACESCoefsLow_4;
            float               ACESCoefsHigh_4;
            float               ACESSceneColorMultiplier;
		};

        struct Params
        {
            D_MATH::Vector3     ColorScale = D_MATH::Vector3::One;
            D_MATH::Vector4     OverlayColor = D_MATH::Vector4::Zero;
            float               DisplayGamma = 2.2f;
            float               ToneMapperGamma = 2.2f;
            bool                IsTemperatureWhiteBalance = true;
            uint32_t            LUTSize = 32u;
            float               WhiteTemp = 6500.f;
            float               WhiteTint = 0.f;
            uint32_t            OutputGamut = 0;
            D_MATH::Vector4     ColorSaturation = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorContrast = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGamma = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGain = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorOffset = D_MATH::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
            D_MATH::Vector4     ColorSaturationShadows = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorContrastShadows = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGammaShadows = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGainShadows = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorOffsetShadows = D_MATH::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
            D_MATH::Vector4     ColorSaturationMidtones = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorContrastMidtones = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGammaMidtones = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGainMidtones = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorOffsetMidtones = D_MATH::Vector4(0.f, 0.0f, 0.0f, 0.0f);
            D_MATH::Vector4     ColorSaturationHighlights = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorContrastHighlights = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGammaHighlights = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorGainHighlights = D_MATH::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Vector4     ColorOffsetHighlights = D_MATH::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
            float               ColorCorrectionShadowsMax = 0.09f;
            float               ColorCorrectionHighlightsMin = 0.5f;
            float               ColorCorrectionHighlightsMax = 1.0f;
            float               BlueCorrection = 0.6f;
            float               ExpandGamut = 1.0f;
            float               ToneCurveAmount = 0.f;
            D_MATH::Vector4     ACESMinMaxData = D_MATH::Vector4(2.50898620e-06f, 1e-4f, 692.651123f, 1000.0f);
            D_MATH::Vector4     ACESMidData = D_MATH::Vector4(0.0822144598f, 4.80000019f, 1.55f, 1.0f);
            D_MATH::Vector4     ACESCoefsLow_0 = D_MATH::Vector4(-4.00000000f, -4.00000000f, -3.15737653f, -0.485249996f);
            D_MATH::Vector4     ACESCoefsHigh_0 = D_MATH::Vector4(-0.332863420f, 1.69534576f, 2.75812411f, 3.00000000f);
            float               ACESCoefsLow_4 = 1.84773231f;
            float               ACESCoefsHigh_4 = 3.0f;
            float               ACESSceneColorMultiplier = 1.5f;

            ShaderParams MakeShaderParams()
            {
                return ShaderParams
                {
                    .ColorScale = ColorScale,
                    .OverlayColor = OverlayColor,
                    .InverseGamma = D_MATH::Vector3(1.f / DisplayGamma, 2.2f / DisplayGamma, 1.f / ToneMapperGamma),
                    .InverseBufferSize = D_MATH::Vector2(1.f / LUTSize, 1.f / LUTSize),
                    .IsTemperatureWhiteBalance = (uint32_t)IsTemperatureWhiteBalance,
                    .LUTSize = (float)LUTSize,
                    .WhiteTemp = WhiteTemp,
                    .WhiteTint = WhiteTint,
                    .OutputGamut = OutputGamut,
                    .ColorSaturation = ColorSaturation,
                    .ColorContrast = ColorContrast,
                    .ColorGamma = ColorGamma,
                    .ColorGain = ColorGain,
                    .ColorOffset = ColorOffset,
                    .ColorSaturationShadows = ColorSaturationShadows,
                    .ColorContrastShadows = ColorContrastShadows,
                    .ColorGammaShadows = ColorGammaShadows,
                    .ColorGainShadows = ColorGainShadows,
                    .ColorOffsetShadows = ColorOffsetShadows,
                    .ColorSaturationMidtones = ColorSaturationMidtones,
                    .ColorContrastMidtones = ColorContrastMidtones,
                    .ColorGammaMidtones = ColorGammaMidtones,
                    .ColorGainMidtones = ColorGainMidtones,
                    .ColorOffsetMidtones = ColorOffsetMidtones,
                    .ColorSaturationHighlights = ColorSaturationHighlights,
                    .ColorContrastHighlights = ColorContrastHighlights,
                    .ColorGammaHighlights = ColorGammaHighlights,
                    .ColorGainHighlights = ColorGainHighlights,
                    .ColorOffsetHighlights = ColorOffsetHighlights,
                    .ColorCorrectionShadowsMax = ColorCorrectionShadowsMax,
                    .ColorCorrectionHighlightsMin = ColorCorrectionHighlightsMin,
                    .ColorCorrectionHighlightsMax = ColorCorrectionHighlightsMax,
                    .BlueCorrection = BlueCorrection,
                    .ExpandGamut = ExpandGamut,
                    .ToneCurveAmount = ToneCurveAmount,
                    .ACESMinMaxData = ACESMinMaxData,
                    .ACESMidData = ACESMidData,
                    .ACESCoefsLow_0 = ACESCoefsLow_0,
                    .ACESCoefsHigh_0 = ACESCoefsHigh_0,
                    .ACESCoefsLow_4 = ACESCoefsLow_4,
                    .ACESCoefsHigh_4 = ACESCoefsHigh_4,
                    .ACESSceneColorMultiplier = ACESSceneColorMultiplier,

                };
            }
        };

        ~ComputeColorLUT() { LUT.Destroy(); }

        INLINE void SetParams(Params const& params)
        {
            Parameters = params;
            LUTDirty = true;
        }

        INLINE Params const& GetParams() const { return Parameters; }

        void                                    ComputeLut(ToneMapperCommonConstants const& toneMapeerConstants, WorkingColorSpaceShaderParameters const& workingColorSpace, Darius::Graphics::ComputeContext& context, Darius::Graphics::Utils::RootSignature const& rs, bool force = false);
        D3D12_CPU_DESCRIPTOR_HANDLE             ComputeAndGetLutSrv(ToneMapperCommonConstants const& toneMapeerConstants, WorkingColorSpaceShaderParameters const& workingColorSpace, Darius::Graphics::ComputeContext& context, Darius::Graphics::Utils::RootSignature const& rs);

        D3D12_CPU_DESCRIPTOR_HANDLE             GetLutSrv() const { return LUT.GetSRV(); }

        private:
        Params                                  Parameters;
        D_GRAPHICS_BUFFERS::ColorBuffer         LUT;
        bool                                    LUTDirty = true;
	};
}