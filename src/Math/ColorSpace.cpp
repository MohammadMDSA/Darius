#include "pch.hpp"

#include "ColorSpace.hpp"
#include "Bounds/BoundingPlane.hpp"

#include <Utils/Assert.hpp>

using namespace D_CONTAINERS;
using namespace D_MATH_BOUNDS;

namespace Darius::Math
{

	static bool bIsWorkingColorSpaceReadyForUse = false;
	static ColorSpace WorkingColorSpace = ColorSpace(ColorSpaceType::sRGB);

	const ColorSpace& ColorSpace::GetWorking()
	{
		return WorkingColorSpace;
	}

	void ColorSpace::SetWorking(ColorSpace ColorSpace)
	{
		WorkingColorSpace = std::move(ColorSpace);
		bIsWorkingColorSpaceReadyForUse = true;
	}

	static bool IsSRGBChromaticities(const DArray<Vector2, 4>& Chromaticities, float Tolerance = 1.e-7)
	{
		return	Chromaticities[0].NearEquals(Vector2(0.64f, 0.33f), Tolerance) &&
			Chromaticities[1].NearEquals(Vector2(0.30f, 0.60f), Tolerance) &&
			Chromaticities[2].NearEquals(Vector2(0.15f, 0.06f), Tolerance) &&
			Chromaticities[3].NearEquals(Vector2(0.3127f, 0.3290f), Tolerance);
	}

	ColorSpace::ColorSpace(const Vector2& InRed, const Vector2& InGreen, const Vector2& InBlue, const Vector2& InWhite)
	{
		mChromaticities[0] = InRed;
		mChromaticities[1] = InGreen;
		mChromaticities[2] = InBlue;
		mChromaticities[3] = InWhite;

		mIsSRGB = IsSRGBChromaticities(mChromaticities);

		mRgbToXYZ = CalcRgbToXYZ();
		mXYZToRgb = mRgbToXYZ.Inverse();
	}

	ColorSpace::ColorSpace(ColorSpaceType ColorSpaceType)
		: mChromaticities(MakeChromaticities(ColorSpaceType))
		, mIsSRGB(ColorSpaceType == ColorSpaceType::sRGB)
	{
		mRgbToXYZ = CalcRgbToXYZ();
		mXYZToRgb = mRgbToXYZ.Inverse();
	}

	DArray<Vector2, 4> ColorSpace::MakeChromaticities(ColorSpaceType ColorSpaceType)
	{
		DArray<Vector2, 4> Chromaticities;

		switch(ColorSpaceType)
		{
		case ColorSpaceType::None:
			// leaves Vector2::Zero from the constructor
			break;
		case ColorSpaceType::sRGB:
			Chromaticities[0] = Vector2(0.64f, 0.33f);
			Chromaticities[1] = Vector2(0.30f, 0.60f);
			Chromaticities[2] = Vector2(0.15f, 0.06f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::Rec2020:
			Chromaticities[0] = Vector2(0.708f, 0.292f);
			Chromaticities[1] = Vector2(0.170f, 0.797f);
			Chromaticities[2] = Vector2(0.131f, 0.046f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::ACESAP0:
			Chromaticities[0] = Vector2(0.7347f, 0.2653f);
			Chromaticities[1] = Vector2(0.0000f, 1.0000f);
			Chromaticities[2] = Vector2(0.0001f, -0.0770f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::ACES_D60);
			break;
		case ColorSpaceType::ACESAP1:
			Chromaticities[0] = Vector2(0.713f, 0.293f);
			Chromaticities[1] = Vector2(0.165f, 0.830f);
			Chromaticities[2] = Vector2(0.128f, 0.044f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::ACES_D60);
			break;
		case ColorSpaceType::P3DCI:
			Chromaticities[0] = Vector2(0.680f, 0.320f);
			Chromaticities[1] = Vector2(0.265f, 0.690f);
			Chromaticities[2] = Vector2(0.150f, 0.060f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::DCI_CalibrationWhite);
			break;
		case ColorSpaceType::P3D65:
			Chromaticities[0] = Vector2(0.680f, 0.320f);
			Chromaticities[1] = Vector2(0.265f, 0.690f);
			Chromaticities[2] = Vector2(0.150f, 0.060f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::REDWideGamut:
			Chromaticities[0] = Vector2(0.780308f, 0.304253f);
			Chromaticities[1] = Vector2(0.121595f, 1.493994f);
			Chromaticities[2] = Vector2(0.095612f, -0.084589f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::SonySGamut3:
			Chromaticities[0] = Vector2(0.730f, 0.280f);
			Chromaticities[1] = Vector2(0.140f, 0.855f);
			Chromaticities[2] = Vector2(0.100f, -0.050f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::SonySGamut3Cine:
			Chromaticities[0] = Vector2(0.766f, 0.275f);
			Chromaticities[1] = Vector2(0.225f, 0.800f);
			Chromaticities[2] = Vector2(0.089f, -0.087f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::AlexaWideGamut:
			Chromaticities[0] = Vector2(0.684f, 0.313f);
			Chromaticities[1] = Vector2(0.221f, 0.848f);
			Chromaticities[2] = Vector2(0.0861f, -0.1020f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::CanonCinemaGamut:
			Chromaticities[0] = Vector2(0.740f, 0.270f);
			Chromaticities[1] = Vector2(0.170f, 1.140f);
			Chromaticities[2] = Vector2(0.080f, -0.100f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::GoProProtuneNative:
			Chromaticities[0] = Vector2(0.698448f, 0.193026f);
			Chromaticities[1] = Vector2(0.329555f, 1.024597f);
			Chromaticities[2] = Vector2(0.108443f, -0.034679f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::PanasonicVGamut:
			Chromaticities[0] = Vector2(0.730f, 0.280f);
			Chromaticities[1] = Vector2(0.165f, 0.840f);
			Chromaticities[2] = Vector2(0.100f, -0.030f);
			Chromaticities[3] = GetWhitePoint(WhitePoint::CIE1931_D65);
			break;
		case ColorSpaceType::PLASA_E1_54:
			Chromaticities[0] = Vector2(0.7347f, 0.2653f);
			Chromaticities[1] = Vector2(0.1596f, 0.8404f);
			Chromaticities[2] = Vector2(0.0366f, 0.0001f);
			Chromaticities[3] = Vector2(0.4254f, 0.4044f);
			break;
		default:
			D_ASSERT_NOENTRY();
			break;
		}

		return Chromaticities;
	}

	Matrix4 ColorSpace::CalcRgbToXYZ() const
	{
		Matrix4 Mat = Matrix4(
			Vector3(mChromaticities[0].GetX(), mChromaticities[0].GetY(), 1.f - mChromaticities[0].GetX() - mChromaticities[0].GetY()),
			Vector3(mChromaticities[1].GetX(), mChromaticities[1].GetY(), 1.f - mChromaticities[1].GetX() - mChromaticities[1].GetY()),
			Vector3(mChromaticities[2].GetX(), mChromaticities[2].GetY(), 1.f - mChromaticities[2].GetX() - mChromaticities[2].GetY()),
			Vector3::Zero
		);

		Matrix4 Inverse = Mat.Inverse();
		const Vector2& White = mChromaticities[3];
		Vector3 WhiteXYZ = Vector3(White.GetX() / White.GetY(), 1.f, (1.f - White.GetX() - White.GetY()) / White.GetY());
		Vector3 Scale = Vector3(Inverse * WhiteXYZ);

		for(unsigned i = 0; i < 3; i++)
		{
			Mat.GetElement(0, i) *= Scale.GetX();
			Mat.GetElement(1, i) *= Scale.GetY();
			Mat.GetElement(2, i) *= Scale.GetZ();
		}
		return Mat;
	}

	bool ColorSpace::Equals(const ColorSpace& ColorSpace, float Tolerance) const
	{
		return Equals(ColorSpace.mChromaticities, Tolerance);
	}

	bool ColorSpace::Equals(const DArray<Vector2, 4>& InChromaticities, float Tolerance) const
	{
		return mChromaticities[0].NearEquals(InChromaticities[0], Tolerance) &&
			mChromaticities[1].NearEquals(InChromaticities[1], Tolerance) &&
			mChromaticities[2].NearEquals(InChromaticities[2], Tolerance) &&
			mChromaticities[3].NearEquals(InChromaticities[3], Tolerance);
	}

	bool ColorSpace::IsSRGB() const
	{
		return mIsSRGB;
	}

	Color ColorSpace::MakeFromColorTemperature(float Temp) const
	{
		Temp = Clamp(Temp, 1000.0f, 15000.0f);

		// Approximate Planckian locus in CIE 1960 UCS
		float u = (0.860117757f + 1.54118254e-4f * Temp + 1.28641212e-7f * Temp * Temp) / (1.f + 8.42420235e-4f * Temp + 7.08145163e-7f * Temp * Temp);
		float v = (0.317398726f + 4.22806245e-5f * Temp + 4.20481691e-8f * Temp * Temp) / (1.f - 2.89741816e-5f * Temp + 1.61456053e-7f * Temp * Temp);

		float x = 3.f * u / (2.f * u - 8.f * v + 4.f);
		float y = 2.f * v / (2.f * u - 8.f * v + 4.f);
		float z = 1.f - x - y;

		Vector3 XYZ = Vector3(1.f / y * x, 1.f, 1.f / y * z);
		Vector4 RGB = mXYZToRgb * XYZ;

		return Color((float)RGB.GetX(), (float)RGB.GetY(), (float)RGB.GetZ());
	}

	float ColorSpace::GetLuminance(const Color& Color) const
	{
		//Note: Equivalent to the dot product of Color and RgbToXYZ.GetColumn(1).
		return Color.GetR() * mRgbToXYZ.GetElement(0, 1) + Color.GetG() * mRgbToXYZ.GetElement(1, 1) + Color.GetB() * mRgbToXYZ.GetElement(2, 1);
	}

	Color ColorSpace::GetLuminanceFactors() const
	{
		return Color(mRgbToXYZ.GetColumn(1));
	}

	static Matrix4 CalcColorSpaceTransformMatrix(const ColorSpace& Src, const ColorSpace& Dst, ChromaticAdaptationMethod Method)
	{
		if(Method == ChromaticAdaptationMethod::None)
		{
			return Src.GetRgbToXYZ() * Dst.GetXYZToRgb();
		}

		const Vector3 SrcWhite = Vector3(Src.GetRgbToXYZ() * Vector3::One);
		const Vector3 DstWhite = Vector3(Dst.GetRgbToXYZ() * Vector3::One);

		if(SrcWhite.NearEquals(DstWhite, 1.e-7f))
		{
			return Src.GetRgbToXYZ() * Dst.GetXYZToRgb();
		}

		Matrix4 ChromaticAdaptationMat = ColorSpaceTransform::CalcChromaticAdaptionMatrix(SrcWhite, DstWhite, Method);
		return Src.GetRgbToXYZ() * ChromaticAdaptationMat * Dst.GetXYZToRgb();
	}

	ColorSpaceTransform::ColorSpaceTransform(ColorSpace const& Src, ColorSpace const& Dst, ChromaticAdaptationMethod Method) :
		Matrix4(CalcColorSpaceTransformMatrix(Src, Dst, Method))
	{

	}

	ColorSpaceTransform::ColorSpaceTransform(Matrix4 const& Matrix) :
		Matrix4(Matrix)
	{ }

	Color ColorSpaceTransform::Apply(const Color& InColor) const
	{
		return *this * InColor;
	}

	Matrix4 ColorSpaceTransform::CalcChromaticAdaptionMatrix(Vector3 const& SourceXYZ, Vector3 const& TargetXYZ, ChromaticAdaptationMethod Method)
	{
		Matrix4 XyzToRgb;

		if(Method == ChromaticAdaptationMethod::CAT02)
		{
			XyzToRgb = Matrix4(
				Plane(0.7328f, 0.4296f, -0.1624f, 0.f),
				Plane(-0.7036f, 1.6975f, 0.0061f, 0.f),
				Plane(0.0030f, 0.0136f, 0.9834f, 0.f),
				Plane(0.f, 0.f, 0.f, 1.f)
			).Transpose();
		}
		else if(Method == ChromaticAdaptationMethod::Bradford)
		{
			XyzToRgb = Matrix4(
				Plane(0.8951f, 0.2664f, -0.1614f, 0.f),
				Plane(-0.7502f, 1.7135f, 0.0367f, 0.f),
				Plane(0.0389f, -0.0685f, 1.0296f, 0.f),
				Plane(0.f, 0.f, 0.f, 1.f)
			).Transpose();
		}
		else
		{
			return Matrix4::Identity;
		}

		Vector4 SourceRGB = XyzToRgb * SourceXYZ;
		Vector4 TargetRGB = XyzToRgb * TargetXYZ;

		Vector4 Scale = Vector4(
			TargetRGB.GetX() / SourceRGB.GetX(),
			TargetRGB.GetY() / SourceRGB.GetY(),
			TargetRGB.GetZ() / SourceRGB.GetZ(),
			1.f);

		Matrix4 ScaleMat = Matrix4::Identity;
		ScaleMat.GetElement(0, 0) = Scale.GetX();
		ScaleMat.GetElement(1, 1) = Scale.GetY();
		ScaleMat.GetElement(2, 2) = Scale.GetZ();
		ScaleMat.GetElement(3, 3) = Scale.GetW();

		Matrix4 RgbToXyz = XyzToRgb.Inverse();

		return XyzToRgb * ScaleMat * RgbToXyz;
	}

}
