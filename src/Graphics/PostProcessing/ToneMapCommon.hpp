#pragma once

#include <Math/VectorMath.hpp>
#include <Math/ColorSpace.hpp>
#include <Utils/Common.hpp>

#ifndef D_GRAPHICS_PP
#define D_GRAPHICS_PP Darius::Graphics::PostProcessing
#endif

namespace Darius::Graphics::PostProcessing
{
	ALIGN_DECL_256 struct ToneMapperCommonConstants
	{
		float FilmSlope;
		float FilmToe;
		float FilmShoulder;
		float FilmBlackClip;
		float FilmWhiteClip;
	};

	ALIGN_DECL_256 struct WorkingColorSpaceShaderParameters
	{
		D_MATH::Matrix4 ToXYZ;
		D_MATH::Matrix4 FromXYZ;
		D_MATH::Matrix4 ToAP1;
		D_MATH::Matrix4 FromAP1;
		D_MATH::Matrix4 ToAP0;
		uint32_t IsSRGB;
	};

}
