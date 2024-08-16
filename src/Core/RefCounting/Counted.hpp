#pragma once

#include "Core/MultiThreading/SafeNumeric.hpp"

#include <rttr/rttr_enable.h>
#include <rttr/type.h>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	class SafeRefCount
	{
	public:
		// True on success
		INLINE bool Ref()
		{
			return mCount.Increment() != 0;
		}

		// Non-zero on success
		INLINE UINT RefVal()
		{
			return mCount.Increment();
		}

		// True if must be disposed of
		INLINE bool Unref()
		{
			CheckUnrefSanity();

			return mCount.Decrement() == 0;
		}

		// Zero if must be disposed of
		INLINE bool UnrefVal()
		{
			CheckUnrefSanity();

			return (bool)mCount.Decrement();
		}

		INLINE UINT Get() const
		{
			return mCount.Get();
		}

		INLINE void Init(UINT value = 1)
		{
			mCount.Set(value);
		}

	private:

		INLINE void CheckUnrefSanity()
		{
			D_ASSERT_M(mCount.Get() != 0, "Trying to unref a SafeRefCount which is already zero");
		}

		D_CORE_THREADING::SafeNumeric<UINT>	mCount;
	};

	template<class T>
	class Ref;

	class Counted : NonCopyable
	{
		RTTR_ENABLE()

	public:

		Counted();
		virtual ~Counted() {}

		INLINE bool				IsReferenced() const { return mRefCountInit.Get() != 1; }
		bool					InitRef();
		// Returns false if refcount is at zero and didn't get increased
		bool					Reference();
		bool					Unreference();
		UINT					GetReferenceCount() const;

	protected:
		virtual bool			Release() = 0;

	private:
		template<class T>
		friend class Ref;

		SafeRefCount			mRefCount;
		SafeRefCount			mRefCountInit;
		
	};
}