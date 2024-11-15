#include "pch.hpp"
#include "Rectangle.hpp"

#include "VectorMath.hpp"

#include <rttr/registration.h>

namespace Darius::Math
{
	const Rectangle Rectangle::Zero = Rectangle();

	Rectangle Rectangle::Union(Rectangle const& ra, Rectangle const& rb) noexcept
	{
        const long righta = ra.mX + ra.mWidth;
        const long rightb = rb.mX + rb.mWidth;

        const long bottoma = ra.mY + ra.mHeight;
        const long bottomb = rb.mY + rb.mHeight;

        const int minX = ra.mX < rb.mX ? ra.mX : rb.mX;
        const int minY = ra.mY < rb.mY ? ra.mY : rb.mY;

        const int maxRight = righta > rightb ? righta : rightb;
        const int maxBottom = bottoma > bottomb ? bottoma : bottomb;

        Rectangle result;
        result.mX = minX;
        result.mY = minY;
        result.mWidth = maxRight - minX;
        result.mHeight = maxBottom - minY;
        return result;
	}

}

RTTR_REGISTRATION
{
	rttr::registration::class_<D_MATH::Rectangle>("Darius::Math::Rectangle")
		.property("X", &D_MATH::Rectangle::mX)
		.property("Y", &D_MATH::Rectangle::mY)
		.property("Width", &D_MATH::Rectangle::mWidth)
		.property("Height", &D_MATH::Rectangle::mHeight);

}