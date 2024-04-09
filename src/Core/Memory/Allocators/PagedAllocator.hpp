#pragma once

#include "Core/MultiThreading/SpinLock.hpp"
#include "Core/Memory/Memory.hpp"

#include <Utils/Log.hpp>

#include <type_traits>
#include <typeinfo>

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY_ALLOC

namespace Darius::Core::Memory
{
	template <class T, bool ThreadSafe = false>
	class PagedAllocator {
		T** mPagePool = nullptr;
		T*** mAvailablePool = nullptr;
		uint32_t mPagesAllocated = 0;
		uint32_t mAllocsAvailable = 0;

		uint32_t mPageShift = 0;
		uint32_t mPageMask = 0;
		uint32_t mPageSize = 0;
		D_CORE_THREADING::SpinLock mSpinLock;

	public:
		enum {
			DEFAULT_PAGE_SIZE = 4096
		};

		template <class... Args>
		T* Alloc(Args const&&...args) {
			if (ThreadSafe) {
				mSpinLock.Lock();
			}
			if (mAllocsAvailable == 0) {
				uint32_t pagesUsed = mPagesAllocated;

				mPagesAllocated++;
				mPagePool = (T**)DMemRealloc(mPagePool, sizeof(T*) * mPagesAllocated);
				mAvailablePool = (T***)DMemRealloc(mAvailablePool, sizeof(T**) * mPagesAllocated);

				mPagePool[pagesUsed] = (T*)DMemAlloc(sizeof(T) * mPageSize);
				mAvailablePool[pagesUsed] = (T**)DMemAlloc(sizeof(T*) * mPageSize);

				for (uint32_t i = 0; i < mPageSize; i++) {
					mAvailablePool[0][i] = &mPagePool[pagesUsed][i];
				}
				mAllocsAvailable += mPageSize;
			}

			mAllocsAvailable--;
			T* alloc = mAvailablePool[mAllocsAvailable >> mPageShift][mAllocsAvailable & mPageMask];
			if (ThreadSafe) {
				mSpinLock.Unlock();
			}
			DMemNew_Placement(alloc, T(args...));
			return alloc;
		}

		void Free(T* mem) {
			if (ThreadSafe) {
				mSpinLock.Lock();
			}
			mem->~T();
			mAvailablePool[mAllocsAvailable >> mPageShift][mAllocsAvailable & mPageMask] = mem;
			mAllocsAvailable++;
			if (ThreadSafe) {
				mSpinLock.Unlock();
			}
		}

	private:
		void _Reset(bool allowUnfreed) {
			if (!allowUnfreed || !std::is_trivially_destructible<T>::value) {
				if (mAllocsAvailable < mPagesAllocated * mPageSize)
					return;
			}
			if (mPagesAllocated) {
				for (uint32_t i = 0; i < mPagesAllocated; i++) {
					DMemFree(mPagePool[i]);
					DMemFree(mAvailablePool[i]);
				}
				DMemFree(mPagePool);
				DMemFree(mAvailablePool);
				mPagePool = nullptr;
				mAvailablePool = nullptr;
				mPagesAllocated = 0;
				mAllocsAvailable = 0;
			}
		}

	public:
		void Reset(bool allowUnfreed = false) {
			if (ThreadSafe) {
				mSpinLock.Lock();
			}
			_Reset(allowUnfreed);
			if (ThreadSafe) {
				mSpinLock.Unlock();
			}
		}

		bool IsConfigured() const {
			if (ThreadSafe) {
				mSpinLock.Lock();
			}
			bool result = mPageSize > 0;
			if (ThreadSafe) {
				mSpinLock.Unlock();
			}
			return result;
		}

		void Configure(uint32_t pageSize) {
			if (ThreadSafe) {
				mSpinLock.Lock();
			}

			if (mPagePool != nullptr) //sanity check
				return;

			if (pageSize == 0)
				return;

			mPageSize = D_MATH::RoundUpToPowerOfTwo(pageSize);
			mPageMask = mPageSize - 1;
			mPageShift = D_MATH::GetShiftFromPowerOfTwo(mPageSize);
			if (ThreadSafe) {
				mSpinLock.Unlock();
			}
		}

		// Power of 2 recommended because of alignment with OS page sizes.
		// Even if element is bigger, it's still a multiple and gets rounded to amount of pages.
		PagedAllocator(uint32_t pageSize = DEFAULT_PAGE_SIZE) {
			Configure(pageSize);
		}

		~PagedAllocator() {
			if (ThreadSafe) {
				mSpinLock.Lock();
			}
			bool leaked = mAllocsAvailable < mPagesAllocated * mPageSize;
			if (leaked) {

				if (true) // Leak reporting
				{
					D_LOG_ERROR(std::format("Pages in use exist at exit in PagedAllocator:{}", typeid(T).name()));
				}
			}
			else {
				_Reset(false);
			}
			if (ThreadSafe) {
				mSpinLock.Unlock();
			}
		}
	};

}
