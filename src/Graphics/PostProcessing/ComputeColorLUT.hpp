#pragma once

#include "Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "ToneMapCommon.hpp"

#include <Math/VectorMath.hpp>
#include <Math/Color.hpp>

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
            D_MATH::Color       ColorScale = D_MATH::Color::White;
            D_MATH::Color       OverlayColor = D_MATH::Color::Black;
            float               DisplayGamma = 2.2f;
            float               ToneMapperGamma = 2.2f;
            bool                IsTemperatureWhiteBalance = true;
            uint32_t            LUTSize = 32u;
            float               WhiteTemp = 6500.f;
            float               WhiteTint = 0.f;
            uint32_t            OutputGamut = 0;
            D_MATH::Color       ColorSaturation = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorContrast = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGamma = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGain = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorOffset = D_MATH::Color(0.0f, 0.0f, 0.0f, 0.0f);
            D_MATH::Color       ColorSaturationShadows = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorContrastShadows = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGammaShadows = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGainShadows = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorOffsetShadows = D_MATH::Color(0.0f, 0.0f, 0.0f, 0.0f);
            D_MATH::Color       ColorSaturationMidtones = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorContrastMidtones = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGammaMidtones = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGainMidtones = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorOffsetMidtones = D_MATH::Color(0.f, 0.0f, 0.0f, 0.0f);
            D_MATH::Color       ColorSaturationHighlights = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorContrastHighlights = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGammaHighlights = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorGainHighlights = D_MATH::Color(1.0f, 1.0f, 1.0f, 1.0f);
            D_MATH::Color       ColorOffsetHighlights = D_MATH::Color(0.0f, 0.0f, 0.0f, 0.0f);
            float               ColorCorrectionShadowsMax = 0.09f;
            float               ColorCorrectionHighlightsMin = 0.5f;
            float               ColorCorrectionHighlightsMax = 1.0f;
            float               BlueCorrection = 0.6f;
            float               ExpandGamut = 1.0f;
            float               ToneCurveAmount = 0.f;
            D_MATH::Color       ACESMinMaxData = D_MATH::Color(2.50898620e-06f, 1e-4f, 692.651123f, 1000.0f);
            D_MATH::Color       ACESMidData = D_MATH::Color(0.0822144598f, 4.80000019f, 1.55f, 1.0f);
            D_MATH::Color       ACESCoefsLow_0 = D_MATH::Color(-4.00000000f, -4.00000000f, -3.15737653f, -0.485249996f);
            D_MATH::Color       ACESCoefsHigh_0 = D_MATH::Color(-0.332863420f, 1.69534576f, 2.75812411f, 3.00000000f);
            float               ACESCoefsLow_4 = 1.84773231f;
            float               ACESCoefsHigh_4 = 3.0f;
            float               ACESSceneColorMultiplier = 1.5f;

            ShaderParams MakeShaderParams()
            {
                return ShaderParams
                {
                    .ColorScale = ColorScale.Vector3(),
                    .OverlayColor = OverlayColor.Vector4(),
                    .InverseGamma = D_MATH::Vector3(1.f / DisplayGamma, 2.2f / DisplayGamma, 1.f / ToneMapperGamma),
                    .InverseBufferSize = D_MATH::Vector2(1.f / LUTSize, 1.f / LUTSize),
                    .IsTemperatureWhiteBalance = (uint32_t)IsTemperatureWhiteBalance,
                    .LUTSize = (float)LUTSize,
                    .WhiteTemp = WhiteTemp,
                    .WhiteTint = WhiteTint,
                    .OutputGamut = OutputGamut,
                    .ColorSaturation = ColorSaturation.Vector4(),
                    .ColorContrast = ColorContrast.Vector4(),
                    .ColorGamma = ColorGamma.Vector4(),
                    .ColorGain = ColorGain.Vector4(),
                    .ColorOffset = ColorOffset.Vector4(),
                    .ColorSaturationShadows = ColorSaturationShadows.Vector4(),
                    .ColorContrastShadows = ColorContrastShadows.Vector4(),
                    .ColorGammaShadows = ColorGammaShadows.Vector4(),
                    .ColorGainShadows = ColorGainShadows.Vector4(),
                    .ColorOffsetShadows = ColorOffsetShadows.Vector4(),
                    .ColorSaturationMidtones = ColorSaturationMidtones.Vector4(),
                    .ColorContrastMidtones = ColorContrastMidtones.Vector4(),
                    .ColorGammaMidtones = ColorGammaMidtones.Vector4(),
                    .ColorGainMidtones = ColorGainMidtones.Vector4(),
                    .ColorOffsetMidtones = ColorOffsetMidtones.Vector4(),
                    .ColorSaturationHighlights = ColorSaturationHighlights.Vector4(),
                    .ColorContrastHighlights = ColorContrastHighlights.Vector4(),
                    .ColorGammaHighlights = ColorGammaHighlights.Vector4(),
                    .ColorGainHighlights = ColorGainHighlights.Vector4(),
                    .ColorOffsetHighlights = ColorOffsetHighlights.Vector4(),
                    .ColorCorrectionShadowsMax = ColorCorrectionShadowsMax,
                    .ColorCorrectionHighlightsMin = ColorCorrectionHighlightsMin,
                    .ColorCorrectionHighlightsMax = ColorCorrectionHighlightsMax,
                    .BlueCorrection = BlueCorrection,
                    .ExpandGamut = ExpandGamut,
                    .ToneCurveAmount = ToneCurveAmount,
                    .ACESMinMaxData = ACESMinMaxData.Vector4(),
                    .ACESMidData = ACESMidData.Vector4(),
                    .ACESCoefsLow_0 = ACESCoefsLow_0.Vector4(),
                    .ACESCoefsHigh_0 = ACESCoefsHigh_0.Vector4(),
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