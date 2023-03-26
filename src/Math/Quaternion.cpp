#include "Quaternion.hpp"

#include <rttr/registration.h>

#include "Quaternion.sgenerated.hpp"

using namespace DirectX;

namespace Darius::Math
{
	const Quaternion Quaternion::Identity = Quaternion(kIdentity);

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
		.property("X", &D_MATH::Quaternion::GetX, &D_MATH::Quaternion::SetX)
		.property("Y", &D_MATH::Quaternion::GetY, &D_MATH::Quaternion::SetY)
		.property("Z", &D_MATH::Quaternion::GetZ, &D_MATH::Quaternion::SetZ)
		.property("W", &D_MATH::Quaternion::GetW, &D_MATH::Quaternion::SetW);
}