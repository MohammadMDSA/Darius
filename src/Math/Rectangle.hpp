#pragma once
#include "pch.hpp"

#include "Vector.hpp"

#include "Rectangle.generated.hpp"

#ifndef D_MATH
#define D_MATH Darius::Math
#endif // !D_MATH


namespace Darius::Math
{
	class DClass() Rectangle
	{
	public:
		Rectangle() noexcept : mX(0), mY(0), mWidth(0), mHeight(0) {}
		constexpr Rectangle(long ix, long iy, long iw, long ih) noexcept : mX(ix), mY(iy), mWidth(iw), mHeight(ih) {}
		explicit Rectangle(const RECT& rct) noexcept : mX(rct.left), mY(rct.top), mWidth(rct.right - rct.left), mHeight(rct.bottom - rct.top) {}

		Rectangle(const Rectangle&) = default;
		Rectangle& operator=(const Rectangle&) = default;

		Rectangle(Rectangle&&) = default;
		Rectangle& operator=(Rectangle&&) = default;

		operator RECT() noexcept { RECT rct; rct.left = mX; rct.top = mY; rct.right = (mX + mWidth); rct.bottom = (mY + mHeight); return rct; }


		bool operator == (const Rectangle& r) const noexcept { return (mX == r.mX) && (mY == r.mY) && (mWidth == r.mWidth) && (mHeight == r.mHeight); }
		bool operator != (const Rectangle& r) const noexcept { return (mX != r.mX) || (mY != r.mY) || (mWidth != r.mWidth) || (mHeight != r.mHeight); }

		bool operator == (const RECT& rct) const noexcept { return (mX == rct.left) && (mY == rct.top) && (mWidth == (rct.right - rct.left)) && (mHeight == (rct.bottom - rct.top)); }
		bool operator != (const RECT& rct) const noexcept { return (mX != rct.left) || (mY != rct.top) || (mWidth != (rct.right - rct.left)) || (mHeight != (rct.bottom - rct.top)); }

		// Assignment operators
		Rectangle& operator=(_In_ const RECT& rct) noexcept { mX = rct.left; mY = rct.top; mWidth = (rct.right - rct.left); mHeight = (rct.bottom - rct.top); return *this; }

		// Rectangle operations
		Vector2 Location() const noexcept;
		Vector2 Center() const noexcept;

		bool IsEmpty() const noexcept { return (mWidth == 0 && mHeight == 0 && mX == 0 && mY == 0); }

		bool Contains(long ix, long iy) const noexcept { return (mX <= ix) && (ix < (mX + mWidth)) && (mY <= iy) && (iy < (mY + mHeight)); }
		bool Contains(const Vector2& point) const noexcept;
		bool Contains(const Rectangle& r) const noexcept { return (mX <= r.mX) && ((r.mX + r.mWidth) <= (mX + mWidth)) && (mY <= r.mY) && ((r.mY + r.mHeight) <= (mY + mHeight)); }
		bool Contains(const RECT& rct) const noexcept { return (mX <= rct.left) && (rct.right <= (mX + mWidth)) && (mY <= rct.top) && (rct.bottom <= (mY + mHeight)); }

		void Inflate(long horizAmount, long vertAmount) noexcept;

		bool Intersects(const Rectangle& r) const noexcept { return (r.mX < (mX + mWidth)) && (mX < (r.mX + r.mWidth)) && (r.mY < (mY + mHeight)) && (mY < (r.mY + r.mHeight)); }
		bool Intersects(const RECT& rct) const noexcept { return (rct.left < (mX + mWidth)) && (mX < rct.right) && (rct.top < (mY + mHeight)) && (mY < rct.bottom); }

		void Offset(long ox, long oy) noexcept { mX += ox; mY += oy; }

		// Static functions
		static Rectangle Intersect(const Rectangle& ra, const Rectangle& rb) noexcept;
		static RECT Intersect(const RECT& rcta, const RECT& rctb) noexcept;

		static Rectangle Union(const Rectangle& ra, const Rectangle& rb) noexcept;
		static RECT Union(const RECT& rcta, const RECT& rctb) noexcept;

		INLINE long GetX() const { return mX; }
		INLINE long GetY() const { return mY; }
		INLINE long GetWidth() const { return mWidth; }
		INLINE long GetHeight() const { return mHeight; }
		INLINE void SetX(long x) { mX = x; }
		INLINE void SetY(long y) { mY = y; }
		INLINE void SetWidth(long w) { mWidth = w; }
		INLINE void SetHeight(long h) { mHeight = h; }

		static const Rectangle Zero;

	private:
		RTTR_REGISTRATION_FRIEND;

		long mX, mY;
		long mWidth, mHeight;
	};
}