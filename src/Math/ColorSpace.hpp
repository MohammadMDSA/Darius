// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Color.hpp"
#include "VectorMath.hpp"

#include <Core/Containers/Array.hpp>

#ifndef D_MATH
#define D_MATH Darius::Math
#endif

namespace Darius::Math
{
	
	enum class ColorSpaceType : uint8_t
	{
		None = 0,
		sRGB = 1,
		Rec2020 = 2,
		ACESAP0 = 3,
		ACESAP1 = 4,
		P3DCI = 5,
		P3D65 = 6,
		REDWideGamut = 7,
		SonySGamut3 = 8,
		SonySGamut3Cine = 9,
		AlexaWideGamut = 10,
		CanonCinemaGamut = 11,
		GoProProtuneNative = 12,
		PanasonicVGamut = 13,
		PLASA_E1_54 = 14,
		Max,
	};

	/** List of available chromatic adaptation methods.
	*
	* NOTE: This list is replicated as a UENUM in TextureDefines.h, and both should always match.
	*/
	enum class ChromaticAdaptationMethod : uint8_t
	{
		None = 0,
		Bradford = 1,
		CAT02 = 2,
		Max,
	};

	/** List of standard white points. */
	enum class WhitePoint : uint8_t
	{
		CIE1931_D65 = 0,
		ACES_D60 = 1,
		DCI_CalibrationWhite,
		Max,
	};

	inline Vector2 GetWhitePoint(WhitePoint InWhitePoint)
	{
		switch(InWhitePoint)
		{
		case WhitePoint::CIE1931_D65:
			return Vector2(0.3127f, 0.3290f);
		case WhitePoint::ACES_D60:
			return Vector2(0.32168f, 0.33767f);
		case WhitePoint::DCI_CalibrationWhite:
			return Vector2(0.314f, 0.351f);

		default:
			D_ASSERT(false);
			return Vector2();
		}
	}

	/** Color space definition as 4 chromaticity coordinates. */
	class ColorSpace
	{
	public:
		/**
		 * Get the global engine working color space (as a singleton).
		 *
		 * @return ColorSpace working color space
		 */
		static const ColorSpace& GetWorking();

		/**
		 * Set the global engine working color space (as a singleton).
		 *
		 * @param ColorSpace working color space
		 */
		static void SetWorking(ColorSpace ColorSpace);


		/** Constructor */
		ColorSpace() { }

		/**
		* Constructor
		*
		* @param InRed Chromaticity 2D coordinates for the red color.
		* @param InGreen Chromaticity 2D coordinates for the green color.
		* @param InBlue Chromaticity 2D coordinates for the blue color.
		* @param InWhite Chromaticity 2D coordinates for the white point.
		*/
		explicit ColorSpace(Vector2 const& InRed, Vector2 const& InGreen, Vector2 const& InBlue, Vector2 const& InWhite);

		/**
		* Constructor
		*
		* @param ColorSpaceType Color space type.
		*/
		explicit ColorSpace(ColorSpaceType ColorSpaceType);

		ColorSpace(ColorSpace&&) = default;
		ColorSpace(const ColorSpace&) = default;
		ColorSpace& operator=(ColorSpace&&) = default;
		ColorSpace& operator=(const ColorSpace&) = default;

		/**
		* Make the chromaticities of the color space type.
		*
		* @return TStaticArray of four chromaticities
		*/
		static D_CONTAINERS::DArray<Vector2, 4> MakeChromaticities(ColorSpaceType ColorSpaceType);

		/**
		* Gets the color space's red chromaticity coordinates.
		*
		* @return Vector2 xy coordinates.
		*/
		inline Vector2 const& GetRedChromaticity() const
		{
			return mChromaticities[0];
		}

		/**
		* Gets the color space's green chromaticity coordinates.
		*
		* @return Vector2 xy coordinates.
		*/
		inline Vector2 const& GetGreenChromaticity() const
		{
			return mChromaticities[1];
		}

		/**
		* Gets the color space's blue chromaticity coordinates.
		*
		* @return Vector2 xy coordinates.
		*/
		inline Vector2 const& GetBlueChromaticity() const
		{
			return mChromaticities[2];
		}

		/**
		* Gets the color space's white point chromaticity coordinates.
		*
		* @return Vector2 xy coordinates.
		*/
		inline Vector2 const& GetWhiteChromaticity() const
		{
			return mChromaticities[3];
		}

		/**
		* Gets the RGB-to-XYZ conversion matrix.
		*
		* @return FMatrix conversion matrix.
		*/
		INLINE const Matrix4& GetRgbToXYZ() const
		{
			return mRgbToXYZ;
		}

		/**
		* Gets the XYZ-to-RGB conversion matrix.
		*
		* @return FMatrix conversion matrix.
		*/
		INLINE const Matrix4& GetXYZToRgb() const
		{
			return mXYZToRgb;
		}

		/**
		 * Check against another color space for equality.
		 *
		 * @param ColorSpace The vector to check against.
		 * @return true if the color spaces are equal, false otherwise.
		 */
		inline bool operator==(const ColorSpace& ColorSpace) const
		{
			return mChromaticities == ColorSpace.mChromaticities;
		}

		/**
		 * Check against another color space for inequality.
		 *
		 * @param ColorSpace The color space to check against.
		 * @return true if the color spaces are not equal, false otherwise.
		 */
		inline bool operator!=(const ColorSpace& ColorSpace) const
		{
			return mChromaticities != ColorSpace.mChromaticities;
		}

		/**
		 * Check against another color space for equality, within specified error limits.
		 *
		 * @param ColorSpace The color space to check against.
		 * @param Tolerance Error tolerance.
		 * @return true if the color spaces are equal within tolerance limits, false otherwise.
		 */
		bool Equals(const ColorSpace& ColorSpace, float Tolerance = 1.e-7f) const;


		/**
		 * Check against color space chromaticities for equality, within specified error limits.
		 *
		 * @param InChromaticities The color space chromaticities to check against.
		 * @param Tolerance Error tolerance.
		 * @return true if the color spaces are equal within tolerance limits, false otherwise.
		 */
		bool Equals(const D_CONTAINERS::DArray<Vector2, 4>& InChromaticities, float Tolerance = 1.e-7f) const;

		/**
		 * Convenience function to verify if the color space matches the engine's default sRGB chromaticities.
		 *
		 * @return true if sRGB.
		 */
		bool IsSRGB() const;

		/**
		* Converts temperature in Kelvins of a black body radiator to an RGB color in the current space.
		*/
		Color MakeFromColorTemperature(float Temp) const;

		/**
		 * Calculate the color's luminance value in the current space.
		 *
		 * @param Color The sampled color.
		 * @return float Luminance value.
		 */
		float GetLuminance(const Color& Color) const;

		/**
		 * Get the luminance factors in the current space.
		 *
		 * @return Color Luminance factors.
		 */
		Color GetLuminanceFactors() const;

	private:

		Matrix4 CalcRgbToXYZ() const;

		/** Red, green, blue, white chromaticities, in order. */
		D_CONTAINERS::DArray<Vector2, 4> mChromaticities;

		Matrix4 mRgbToXYZ;
		Matrix4 mXYZToRgb;

		bool mIsSRGB;

	};

	struct ColorSpaceTransform : Matrix4
	{
		/**
		* Constructor: create a color space transformation matrix from a source to a target color space using the RGB-XYZ-RGB conversions.
		*
		* @param Source Source color space.
		* @param Target Target color space.
		* @param Method Chromatic adapation method.
		*/
		explicit ColorSpaceTransform(const ColorSpace& Src, const ColorSpace& Dst, ChromaticAdaptationMethod Method = ChromaticAdaptationMethod::Bradford);

		/**
		* Constructor: create a color space transformation matrix from a raw matrix.
		*
		* @param Matrix Color space transformation matrix.
		*/
		explicit ColorSpaceTransform(Matrix4 const& Matrix);

		/**
		* Apply color space transform to FLinearColor.
		*
		* @param Color Color to transform.
		*/
		Color Apply(const Color& Color) const;

		/**
		* Calculate the chromatic adaptation matrix using the specific method.
		*
		* @param SourceXYZ Source color in XYZ space.
		* @param TargetXYZ Target color in XYZ space.
		* @param Method Adaptation method (None, Bradford, CAT02).
		*/
		static Matrix4 CalcChromaticAdaptionMatrix(Vector3 const& SourceXYZ, Vector3 const& TargetXYZ, ChromaticAdaptationMethod Method = ChromaticAdaptationMethod::Bradford);
	};

}
