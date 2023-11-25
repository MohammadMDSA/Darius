#include "pch.hpp"
#include "Rectangle.hpp"

#include <rttr/registration.h>

namespace Darius::Math
{
	const Rectangle Rectangle::Zero = Rectangle();
}

RTTR_REGISTRATION
{
	rttr::registration::class_<D_MATH::Rectangle>("Darius::Math::Rectangle")
		.property("X", &D_MATH::Rectangle::mX)
		.property("Y", &D_MATH::Rectangle::mY)
		.property("Width", &D_MATH::Rectangle::mWidth)
		.property("Height", &D_MATH::Rectangle::mHeight);

}