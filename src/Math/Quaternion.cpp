#include "Quaternion.hpp"

#include <rttr/registration.h>

#include "Quaternion.sgenerated.hpp"

using namespace DirectX;

namespace Darius::Math
{
	const Quaternion Quaternion::Identity = Quaternion(kIdentity);
	const Quaternion Quaternion::Inverted = { 0.f, 0.f, 0.f, -1.f };

	Quaternion::Quaternion(Rotator const& rot)
	{
		rot.GetQuaternion(*this);
	}

	Rotator Quaternion::GetRotator() const
	{
		const float X = GetX(), Y = GetY(), Z = GetZ(), W = GetW();
		const float singularityTest = Z * X - W * Y;
		const float yawY = 2.f * (W * Z + X * Y);
		const float yawX = (1.f - 2.f * (Y * Y + Z * Z));

		const float singularityThreshold = 0.49999f;
		const float radToDeg = (180.f / DirectX::XM_PI);
		float pitch, yaw, roll;

		if (singularityTest < -singularityThreshold)
		{
			pitch = -90.f;
			yaw = (atan2f(yawY, yawX) * radToDeg);
			roll = Rotator::NormalizeAxis(-yaw - (2.f * atan2f(X, W) * radToDeg));
		}
		else if (singularityTest > singularityThreshold)
		{
			pitch = 90.f;
			yaw = atan2f(yawY, yawX) * radToDeg;
			roll = Rotator::NormalizeAxis(yaw - (2.f * atan2f(X, W) * radToDeg));
		}
		else
		{
			pitch = asinf(2.f * singularityTest) * radToDeg;
			yaw = atan2f(yawY, yawX) * radToDeg;
			roll = atan2f(-2.f * (W * X + Y * Z), (1.f - 2.f * (X * X + Y * Y))) * radToDeg;
		}

		return Rotator(pitch, yaw, roll);
	}

	Vector3 Quaternion::Angles() const
	{
		Vector3 result;
		auto mat = DirectX::XMMatrixRotationQuaternion(m_vec);
		auto fmat = DirectX::XMFLOAT4X4((float*)&mat);

		if (fmat._32 < 1.f)
		{
			if (fmat._32 > -1.f)
			{
				result.SetX(asinf(fmat._32));
				result.SetZ(atan2f(-fmat._12, fmat._22));
				result.SetY(atan2f(-fmat._31, fmat._33));
			}
			else
			{
				result.SetX(-DirectX::XM_PIDIV2);
				result.SetZ(-atan2f(fmat._13, fmat._11));
				result.SetY(0.f);
			}
		}
		else
		{
			result.SetX(DirectX::XM_PIDIV2);
			result.SetZ(atan2f(fmat._13, fmat._11));
			result.SetY(0.f);
		}
		return -result;
	}

#ifdef _D_EDITOR

	bool DrawDetails(Quaternion& quat, float params[])
	{
		Vector3 radian = quat.Angles();
		Vector3 deg;

		deg.SetX(XMConvertToDegrees(radian.GetX()));
		deg.SetY(XMConvertToDegrees(radian.GetY()));
		deg.SetZ(XMConvertToDegrees(radian.GetZ()));

		float def[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM_VECTOR;
		if (DrawDetails(deg, def))
		{
			quat = Quaternion(XMConvertToRadians(deg.GetX()), XMConvertToRadians(deg.GetY()), XMConvertToRadians(deg.GetZ()));
			return true;
		}
		return false;
	}

#endif // _D_EDITOR

}


RTTR_REGISTRATION
{
	rttr::registration::class_<D_MATH::Quaternion>("Darius::Math::Quaternion")
		.property("X", &D_MATH::Quaternion::GetX, &D_MATH::Quaternion::SetX) (rttr::metadata("NO_SERIALIZE", true))
		.property("Y", &D_MATH::Quaternion::GetY, &D_MATH::Quaternion::SetY) (rttr::metadata("NO_SERIALIZE", true))
		.property("Z", &D_MATH::Quaternion::GetZ, &D_MATH::Quaternion::SetZ) (rttr::metadata("NO_SERIALIZE", true))
		.property("W", &D_MATH::Quaternion::GetW, &D_MATH::Quaternion::SetW) (rttr::metadata("NO_SERIALIZE", true))
		.property("_data", &D_MATH::Quaternion::GetData, &D_MATH::Quaternion::SetData);
}

