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

#pragma once
#include <Utils/Common.hpp>
#include <DirectXMath.h>

#include <rttr/registration_friend.h>
#include <rttr/rttr_enable.h>

#include <Color.generated.hpp>

#ifndef D_MATH
#define D_MATH Darius::Math
#endif

namespace Darius::Math
{
	class DClass(Serialize[R, G, B, A]) Color
	{
		GENERATED_BODY();

	public:
		Color() : m_value(DirectX::g_XMOne) {}
		Color(DirectX::FXMVECTOR vec);
		Color(const DirectX::XMVECTORF32& vec);
		Color(float r, float g, float b, float a = 1.0f);
		Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255, uint16_t bitDepth = 8);
		explicit Color(uint32_t rgbaLittleEndian);

		float GetR() const { return DirectX::XMVectorGetX(m_value); }
		float GetG() const { return DirectX::XMVectorGetY(m_value); }
		float GetB() const { return DirectX::XMVectorGetZ(m_value); }
		float GetA() const { return DirectX::XMVectorGetW(m_value); }

		bool operator==(const Color& rhs) const { return DirectX::XMVector4Equal(m_value, rhs.m_value); }
		bool operator!=(const Color& rhs) const { return !DirectX::XMVector4Equal(m_value, rhs.m_value); }

		void SetR(float r) { m_value.f[0] = r; }
		void SetG(float g) { m_value.f[1] = g; }
		void SetB(float b) { m_value.f[2] = b; }
		void SetA(float a) { m_value.f[3] = a; }

		float* GetPtr(void) { return reinterpret_cast<float*>(&m_value); }
		float& operator[](int idx) { return GetPtr()[idx]; }

		void SetRGB(float r, float g, float b) { m_value.v = DirectX::XMVectorSelect(m_value, DirectX::XMVectorSet(r, g, b, b), DirectX::g_XMMask3); }

		Color ToSRGB() const;
		Color FromSRGB() const;
		Color ToREC709() const;
		Color FromREC709() const;

		// Probably want to convert to sRGB or Rec709 first
		uint32_t R10G10B10A2() const;
		uint32_t R8G8B8A8() const;

		// Pack an HDR color into 32-bits
		uint32_t R11G11B10F(bool RoundToEven = false) const;
		uint32_t R9G9B9E5() const;

		operator DirectX::XMVECTOR() const { return m_value; }
		operator DirectX::XMFLOAT4() const { return DirectX::XMFLOAT4(m_value); }

		static const Color			Red;
		static const Color			Blue;
		static const Color			Green;
		static const Color			Yellow;
		static const Color			White;
		static const Color			Black;

	private:
		DirectX::XMVECTORF32 m_value;
	};

	INLINE Color Max(Color a, Color b) { return Color(DirectX::XMVectorMax(a, b)); }
	INLINE Color Min(Color a, Color b) { return Color(DirectX::XMVectorMin(a, b)); }
	INLINE Color Clamp(Color x, Color a, Color b) { return Color(DirectX::XMVectorClamp(x, a, b)); }


	inline Color::Color(DirectX::FXMVECTOR vec)
	{
		m_value.v = vec;
	}

	inline Color::Color(const DirectX::XMVECTORF32& vec)
	{
		m_value = vec;
	}

	inline Color::Color(float r, float g, float b, float a)
	{
		m_value.v = DirectX::XMVectorSet(r, g, b, a);
	}

	inline Color::Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth)
	{
		m_value.v = DirectX::XMVectorScale(DirectX::XMVectorSet((float)r, (float)g, (float)b, (float)a), 1.0f / (float)((1 << bitDepth) - 1));
	}

	inline Color::Color(uint32_t u32)
	{
		float r = (float)((u32 >> 0) & 0xFF);
		float g = (float)((u32 >> 8) & 0xFF);
		float b = (float)((u32 >> 16) & 0xFF);
		float a = (float)((u32 >> 24) & 0xFF);
		m_value.v = DirectX::XMVectorScale(DirectX::XMVectorSet(r, g, b, a), 1.0f / 255.0f);
	}

	inline Color Color::ToSRGB(void) const
	{
		DirectX::XMVECTOR T = DirectX::XMVectorSaturate(m_value);
		DirectX::XMVECTOR result = DirectX::XMVectorSubtract(DirectX::XMVectorScale(DirectX::XMVectorPow(T, DirectX::XMVectorReplicate(1.0f / 2.4f)), 1.055f), DirectX::XMVectorReplicate(0.055f));
		result = DirectX::XMVectorSelect(result, DirectX::XMVectorScale(T, 12.92f), DirectX::XMVectorLess(T, DirectX::XMVectorReplicate(0.0031308f)));
		return DirectX::XMVectorSelect(T, result, DirectX::g_XMSelect1110);
	}

	inline Color Color::FromSRGB(void) const
	{
		DirectX::XMVECTOR T = DirectX::XMVectorSaturate(m_value);
		DirectX::XMVECTOR result = DirectX::XMVectorPow(DirectX::XMVectorScale(DirectX::XMVectorAdd(T, DirectX::XMVectorReplicate(0.055f)), 1.0f / 1.055f), DirectX::XMVectorReplicate(2.4f));
		result = DirectX::XMVectorSelect(result, DirectX::XMVectorScale(T, 1.0f / 12.92f), DirectX::XMVectorLess(T, DirectX::XMVectorReplicate(0.0031308f)));
		return DirectX::XMVectorSelect(T, result, DirectX::g_XMSelect1110);
	}

	inline Color Color::ToREC709(void) const
	{
		DirectX::XMVECTOR T = DirectX::XMVectorSaturate(m_value);
		DirectX::XMVECTOR result = DirectX::XMVectorSubtract(DirectX::XMVectorScale(DirectX::XMVectorPow(T, DirectX::XMVectorReplicate(0.45f)), 1.099f), DirectX::XMVectorReplicate(0.099f));
		result = DirectX::XMVectorSelect(result, DirectX::XMVectorScale(T, 4.5f), DirectX::XMVectorLess(T, DirectX::XMVectorReplicate(0.0018f)));
		return DirectX::XMVectorSelect(T, result, DirectX::g_XMSelect1110);
	}

	inline Color Color::FromREC709(void) const
	{
		DirectX::XMVECTOR T = XMVectorSaturate(m_value);
		DirectX::XMVECTOR result = DirectX::XMVectorPow(DirectX::XMVectorScale(DirectX::XMVectorAdd(T, DirectX::XMVectorReplicate(0.099f)), 1.0f / 1.099f), DirectX::XMVectorReplicate(1.0f / 0.45f));
		result = DirectX::XMVectorSelect(result, DirectX::XMVectorScale(T, 1.0f / 4.5f), DirectX::XMVectorLess(T, DirectX::XMVectorReplicate(0.0081f)));
		return DirectX::XMVectorSelect(T, result, DirectX::g_XMSelect1110);
	}

	inline uint32_t Color::R10G10B10A2(void) const
	{
		DirectX::XMVECTOR result = DirectX::XMVectorRound(DirectX::XMVectorMultiply(DirectX::XMVectorSaturate(m_value), DirectX::XMVectorSet(1023.0f, 1023.0f, 1023.0f, 3.0f)));
		result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
		uint32_t r = DirectX::XMVectorGetIntX(result);
		uint32_t g = DirectX::XMVectorGetIntY(result);
		uint32_t b = DirectX::XMVectorGetIntZ(result);
		uint32_t a = DirectX::XMVectorGetIntW(result) >> 8;
		return a << 30 | b << 20 | g << 10 | r;
	}

	inline uint32_t Color::R8G8B8A8(void) const
	{
		DirectX::XMVECTOR result = DirectX::XMVectorRound(DirectX::XMVectorMultiply(DirectX::XMVectorSaturate(m_value), DirectX::XMVectorReplicate(255.0f)));
		result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
		uint32_t r = DirectX::XMVectorGetIntX(result);
		uint32_t g = DirectX::XMVectorGetIntY(result);
		uint32_t b = DirectX::XMVectorGetIntZ(result);
		uint32_t a = DirectX::XMVectorGetIntW(result);
		return a << 24 | b << 16 | g << 8 | r;
	}

}