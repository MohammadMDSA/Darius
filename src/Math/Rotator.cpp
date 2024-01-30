#include "Rotator.hpp"
#include "Quaternion.hpp"

#include <rttr/registration.h>

#include "Rotator.sgenerated.hpp"

namespace Darius::Math
{
	const Rotator Rotator::ZeroRotator = Rotator(0.f, 0.f, 0.f);

	Rotator::Rotator(Quaternion const& quat) :
		mPitchYawRoll(quat.GetRotator().mPitchYawRoll)
	{ }

	Rotator Rotator::GetInverse() const
	{
		return Quaternion(*this).Invert().GetRotator();
	}

	Vector3 Rotator::RotateVector(Vector3 const& vec) const
	{
		return Quaternion(*this) * vec;
	}

	Vector3 Rotator::UnrotateVector(Vector3 const& vec) const
	{
		return Quaternion(*this).Invert() * vec;
	}

	Rotator Rotator::Clamp() const
	{
		return Rotator(ClampAxis(mPitchYawRoll.GetX()), ClampAxis(mPitchYawRoll.GetY()), ClampAxis(mPitchYawRoll.GetZ()));
	}
	
	Rotator Rotator::GetNormalized() const
	{
		Rotator r = *this;
		r.Normalize();
		return r;
	}

	Rotator Rotator::GetDenormalized() const
	{
		Rotator r = *this;
		r.Clamp();
		return r;
	}

	void Rotator::GetQuaternion(Quaternion& outQuat) const
	{
		D_ASSERT_M(false, "Not Implemented");
		const Vector4 Angles = Vector4(mPitchYawRoll);
		const Vector4 AnglesNoWinding = Angles - (360.f * Vector4(DirectX::XMVectorRound(Angles / 360.f)));
		const Vector4 HalfAngles = AnglesNoWinding * Vector4((DirectX::XM_PI / 180.f) * 0.5f);

		//DirectX::XMVECTOR _SinAngles, _CosAngles;
		//DirectX::XMVectorSinCos(&_SinAngles, &_CosAngles, HalfAngles);
		//Vector4 SinAngles(_SinAngles), CosAngles(_CosAngles);

		//// Vectorized conversion, measured 20% faster than using scalar version after VectorSinCos.
		//// Indices within VectorRegister (for shuffles): P=0, Y=1, R=2
		//const Vector4 SR = Vector4(SinAngles.GetZ());
		//const Vector4 CR = Vector4(CosAngles.GetZ());

		//const Vector4 SY_SY_CY_CY_Temp = DirectX::shu VectorShuffle(SinAngles, CosAngles, 1, 1, 1, 1);

		//const VectorRegister4Float SP_SP_CP_CP = VectorShuffle(SinAngles, CosAngles, 0, 0, 0, 0);
		//const VectorRegister4Float SY_CY_SY_CY = VectorShuffle(SY_SY_CY_CY_Temp, SY_SY_CY_CY_Temp, 0, 2, 0, 2);

		//const VectorRegister4Float CP_CP_SP_SP = VectorShuffle(CosAngles, SinAngles, 0, 0, 0, 0);
		//const VectorRegister4Float CY_SY_CY_SY = VectorShuffle(SY_SY_CY_CY_Temp, SY_SY_CY_CY_Temp, 2, 0, 2, 0);

		//const uint32 Neg = uint32(1 << 31);
		//const uint32 Pos = uint32(0);
		//const VectorRegister4Float SignBitsLeft = MakeVectorRegister(Pos, Neg, Pos, Pos);
		//const VectorRegister4Float SignBitsRight = MakeVectorRegister(Neg, Neg, Neg, Pos);
		//const VectorRegister4Float LeftTerm = VectorBitwiseXor(SignBitsLeft, VectorMultiply(CR, VectorMultiply(SP_SP_CP_CP, SY_CY_SY_CY)));
		//const VectorRegister4Float RightTerm = VectorBitwiseXor(SignBitsRight, VectorMultiply(SR, VectorMultiply(CP_CP_SP_SP, CY_SY_CY_SY)));

		//const VectorRegister4Float Result = VectorAdd(LeftTerm, RightTerm);
		//FQuat4f RotationQuat = FQuat4f::MakeFromVectorRegister(Result);
	}

	void Rotator::Normalize()
	{
		// shift in the range [-360, 360]
		Vector3 v0 = mPitchYawRoll - (360.f * Vector3(DirectX::XMVectorRound(mPitchYawRoll / 360.f)));
		Vector3 v1 = v0 + Vector3(360.f);
		Vector3 v2 = Vector3(DirectX::XMVectorSelect(v0, v1, DirectX::XMVectorGreaterOrEqual(v0, DirectX::g_XMZero)));

		// shift in the range [-180, 180]
		Vector3 v3 = v2 - Vector3(360.f);
		mPitchYawRoll = Vector3(DirectX::XMVectorSelect(v3, v2, DirectX::XMVectorGreaterOrEqual(v2, Vector3(180.f))));
	}

	float Rotator::ClampAxis(float angle)
	{
		// to (-360, 360)
		angle = fmodf(angle, 360.f);
		
		if (angle < 0.f)
		{
			// to [0, 360)
			angle += 360.f;
		}

		return angle;
	}

	float Rotator::NormalizeAxis(float angle)
	{
		// to [0, 360)
		angle = ClampAxis(angle);

		if (angle > 180.f)
		{
			// to (-180, 180)
			angle -= 360.f;
		}

		return angle;
	}

}

RTTR_REGISTRATION
{
	rttr::registration::class_<D_MATH::Rotator>("Darius::Math::Rotator")
		.property("Pitch", &D_MATH::Rotator::GetPitch, &D_MATH::Rotator::SetPitch)
		.property("Yaw", &D_MATH::Rotator::GetYaw, &D_MATH::Rotator::SetYaw)
		.property("Roll", &D_MATH::Rotator::GetRoll, &D_MATH::Rotator::SetRoll);
}