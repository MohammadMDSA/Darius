#pragma once
#include "pch.hpp"
#include "Counted.hpp"

#include <Core/Exceptions/Exception.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <optional>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	template<class T>
	class Ref
	{
	public:

		Ref() : Ref(nullptr) {}

		Ref(T* data, std::optional<CountedOwner> ownerData = std::nullopt)
		{
			using conv = std::is_convertible<T*, Counted*>;
			D_STATIC_ASSERT(conv::value);

			mData = data;

			if (data == nullptr)
			{
				return;
			}

			if (ownerData.has_value())
			{
				mOwnerData = ownerData.value();
			}
			else
			{
				mOwnerData = CountedOwner();
			}

			data->AddOwner(mOwnerData);
		}

		Ref(T* data, std::wstring const& ownerName, std::string const& ownerType, void* ownerRef) : Ref(data, CountedOwner{ ownerName, ownerType, ownerRef })
		{
		}

		Ref(Ref<T> const& other)
		{
			Unref();
			mData = other.mData;
			mOwnerData = other.mOwnerData;
			mData->AddOwner(mOwnerData);
		}

		INLINE Ref<T>& operator= (Ref<T> const& other)
		{
			Unref();
			mData = other.mData;
			mOwnerData = other.mOwnerData;
			mData->AddOwner(mOwnerData);
			return *this;
		}

		~Ref()
		{
			Unref();
		}

		void Unref()
		{
			if (IsValid())
			{
				mData->RemoveOwner(mOwnerData);
			}
			mData = nullptr;
		}

		INLINE T* Get()
		{
			return mData;
		}

		INLINE const T* Get() const
		{
			return mData;
		}

		INLINE bool IsValid() const
		{
			return mData != nullptr;
		}

		INLINE const T* operator->() const
		{
			if (!IsValid()) throw D_EXCEPTION::NullPointerException();
			return mData;
		}

		INLINE T* operator->()
		{
			if (!IsValid()) throw D_EXCEPTION::NullPointerException();
			return mData;
		}

		INLINE T& operator*()
		{
			return *mData;
		}

	private:
		CountedOwner				mOwnerData;
		T* mData = nullptr;
	};
}
