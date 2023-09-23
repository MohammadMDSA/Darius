#pragma once

#include <Utils/Common.hpp>
#include <Utils/Assert.hpp>

#include <rttr/rttr_enable.h>
#include <rttr/type.h>

#include <atomic>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{

	// This class is heavily inspired from Godot Engine
	template<class T>
	class SafeNumeric
	{
		D_STATIC_ASSERT(std::atomic<T>::is_always_lock_free);

	public:
		INLINE void Set(T value)
		{
			mValue.store(value, std::memory_order_release);
		}

		INLINE T Get() const
		{
			return mValue.load(std::memory_order_acquire);
		}

		// Increment and return the new value
		INLINE T Increment()
		{
			return mValue.fetch_add(1u, std::memory_order_acq_rel) + 1u;
		}

		// Increment and return the previous value
		INLINE T PostIncrement()
		{
			return mValue.fetch_add(1u, std::memory_order_acq_rel);
		}

		// Decrement and return the new value
		INLINE T Decrement()
		{
			return mValue.fetch_sub(1u, std::memory_order_acq_rel) - 1u;
		}

		// Decrement and return the previous value
		INLINE T PostDecrement()
		{
			return mValue.fetch_sub(1u, std::memory_order_acq_rel);
		}

		INLINE T Add(T value)
		{
			return mValue.fetch_add(value, std::memory_order_acq_rel) + value;
		}

		// Returns the original value instead of the new one
		INLINE T PostAdd(T value)
		{
			return mValue.fetch_add(value, std::memory_order_acq_rel);
		}

		INLINE T Sub(T value)
		{
			return mValue.fetch_sub(value, std::memory_order_acq_rel) - value;
		}

		INLINE T BitOr(T value)
		{
			return mValue.fetch_or(value, std::memory_order_acq_rel);
		}
		INLINE T BitAnd(T value)
		{
			return mValue.fetch_and(value, std::memory_order_acq_rel);
		}

		INLINE T BitXor(T value)
		{
			return mValue.fetch_xor(value, std::memory_order_acq_rel);
		}

		// Returns the original value instead of the new one
		INLINE T PostSub(T value)
		{
			return mValue.fetch_sub(value, std::memory_order_acq_rel);
		}

		INLINE T ExchangeIfGreater(T value)
		{
			while (true)
			{
				T tmp = mValue.load(std::memory_order_acquire);
				if (tmp >= value)
				{
					return tmp; // already greater, or equal
				}

				if (mValue.compare_exchange_weak(tmp, value, std::memory_order_acq_rel))
				{
					return value;
				}
			}
		}

		INLINE T ConditionalIncrement()
		{
			while (true)
			{
				T c = mValue.load(std::memory_order_acquire);
				if (c == 0)
				{
					return 0;
				}
				if (mValue.compare_exchange_weak(c, c + 1, std::memory_order_acq_rel))
				{
					return c + 1;
				}
			}
		}

		INLINE explicit SafeNumeric<T>(T value = static_cast<T>(0))
		{
			Set(value);
		}

	private:
		std::atomic<T>			mValue;
	};

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

			return mCount.Decrement();
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

		SafeNumeric<UINT>	mCount;
	};

	template<class T>
	class Ref;

	class Counted : NonCopyable
	{
		RTTR_ENABLE()

	public:

		Counted();
		~Counted() {}

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