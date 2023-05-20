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

		Ref() : Ref(nullptr, std::nullopt) {} 

		Ref(CountedOwner ownerData) : Ref(nullptr, ownerData) {}

		Ref(T* data, std::optional<CountedOwner> ownerData = std::nullopt)
		{
			using conv = std::is_convertible<T*, Counted*>;
			D_STATIC_ASSERT(conv::value);

			mData = data;

			if (ownerData.has_value())
			{
				mOwnerData = ownerData.value();
			}
			else
			{
				mOwnerData = CountedOwner();
			}

			if (data == nullptr)
			{
				return;
			}

			data->AddOwner(mOwnerData);
		}

		Ref(T* data, std::wstring const& ownerName, std::string const& ownerType, void* ownerRef) : Ref(data, CountedOwner{ ownerName, ownerType, ownerRef })
		{
		}

		Ref(Ref<T> const& other)
		{
			Unref();
			if (mOwnerData.Ref == nullptr)
				mOwnerData = other.mOwnerData;
			mData = other.mData;
			if (other.IsValid())
				mData->AddOwner(mOwnerData);
		}

		INLINE Ref<T>& operator= (Ref<T> const& other)
		{
			Unref();
			if (mOwnerData.Ref == nullptr)
				mOwnerData = other.mOwnerData;
			mData = other.mData;
			if (other.IsValid())
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

		// The new function only works after the next assignment
		INLINE void SetChangeCallback(std::function<void()> callback)
		{
			mOwnerData.ChangeCallback = callback;
		}

		INLINE T* Get() const
		{
			return mData;
		}

		INLINE bool IsValid() const
		{
			return mData != nullptr;
		}

		INLINE T* operator->() const
		{
			if (!IsValid()) throw D_EXCEPTION::NullPointerException();
			return mData;
		}

		INLINE T& operator*() const
		{
			return *mData;
		}

	private:
		CountedOwner				mOwnerData;
		T* mData = nullptr;
	};
}
