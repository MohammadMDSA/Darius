#include "Matrix3.hpp"

#include <rttr/registration.h>

#include "Matrix3.sgenerated.hpp"

namespace Darius::Math
{
	const Matrix3 Matrix3::Identity = Matrix3(kIdentity);
	const Matrix3 Matrix3::Zero = Matrix3(kZero);
}

RTTR_REGISTRATION
{
	rttr::registration::class_<D_MATH::Matrix3>("Darius::Math::Matrix3")
		.property("_data", &D_MATH::Matrix3::GetData, &D_MATH::Matrix3::SetData);
}
