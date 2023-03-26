#include "Matrix4.hpp"

#include <rttr/registration.h>

#include "Matrix3.sgenerated.hpp"

namespace Darius::Math
{
	const Matrix4 Matrix4::Identity = Matrix4(kIdentity);
	const Matrix4 Matrix4::Zero = Matrix4(kZero);
}

RTTR_REGISTRATION
{
	rttr::registration::class_<D_MATH::Matrix4>("Darius::Math::Matrix4")
		.property("_data", &D_MATH::Matrix4::GetData, &D_MATH::Matrix4::SetData);
}
