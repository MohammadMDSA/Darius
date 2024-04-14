#pragma once

#include <Utils/Common.hpp>

#include <atomic>

#ifndef D_CORE_THREADING
#define D_CORE_THREADING Darius::Core::MultiThreading
#endif // !D_CORE_THREADING


namespace Darius::Core::MultiThreading
{
	class SpinLock {
		mutable std::atomic_flag mLocked = ATOMIC_FLAG_INIT;

	public:
		INLINE void Lock() const {
			while (mLocked.test_and_set(std::memory_order_acquire)) {
				// Continue.
			}
		}
		INLINE void Unlock() const {
			mLocked.clear(std::memory_order_release);
		}
	};
}