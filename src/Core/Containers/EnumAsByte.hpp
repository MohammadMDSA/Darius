#pragma once

#include "Utils/Common.hpp"

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

#include "EnumAsByte.generated.hpp"

namespace Darius::Core::Containers
{
	template<class ENUM>
	class EnumAsByte
	{
	public:
		typedef ENUM EnumType;

		EnumAsByte() = default;
		EnumAsByte(EnumAsByte const&) = default;
		EnumAsByte& operator=(EnumAsByte const&) = default;

		INLINE EnumAsByte(ENUM value) :
			mValue(static_cast<uint8_t>(value)) {}

		explicit INLINE EnumAsByte(int value) :
			mValue(static_cast<uint8_t>(value)) {}

		explicit INLINE EnumAsByte(uint8_t value) :
			mValue(static_cast<uint8_t>(value)) {}

		bool operator==(ENUM value) const
		{
			return static_cast<ENUM>(mValue) == value;
		}

		bool operator==(EnumAsByte value) const
		{
			return mValue == value.mValue;
		}

		operator ENUM() const
		{
			return (ENUM)mValue;
		}

		ENUM GetValue() const
		{
			return (ENUM)mValue;
		}

		uint8_t GetIntValue() const
		{
			return mValue;
		}

	private:

		uint8_t mValue;
	};
}
