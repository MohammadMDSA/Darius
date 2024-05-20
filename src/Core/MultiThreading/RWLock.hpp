#pragma once

#include <Utils/Common.hpp>

#include <shared_mutex>

#ifndef D_CORE_THREADING
#define D_CORE_THREADING Darius::Core::MultiThreading
#endif // !D_CORE_THREADING


namespace Darius::Core::MultiThreading
{
	class RWLock
	{
	public:

		// Locks as reader. Block if locked by writer
		INLINE void ReadLock() const
		{
			mMutex.lock_shared();
		}

		// Unlock as reader. Let other threads continue
		INLINE void ReadUnlock() const
		{
			mMutex.unlock_shared();
		}

		// Attempt to lock for reading. True on success, false if can't lock
		INLINE bool ReadTryLock() const
		{
			return mMutex.try_lock_shared();
		}

		// Lock as writer. Block if already locked.
		INLINE void WriteLock()
		{
			mMutex.lock();
		}

		// Unlock as writer, let others continue
		INLINE void WriterUnlock()
		{
			mMutex.unlock();
		}

		// Attempt to lock as writer. True on success, false if can't lock
		INLINE bool WriterTryLock()
		{
			return mMutex.try_lock();
		}

	private:
		mutable std::shared_timed_mutex mMutex;
	};

	class RWLockRead
	{
	public:
		INLINE RWLockRead(RWLock const& lock) :
			mLock(lock)
		{
			mLock.ReadLock();
		}

		INLINE ~RWLockRead()
		{
			mLock.ReadUnlock();
		}

	private:
		RWLock const& mLock;
	};

	class RWLockWrite
	{
	public:
		INLINE RWLockWrite(RWLock& lock) :
			mLock(lock)
		{
			mLock.WriteLock();
		}

		INLINE ~RWLockWrite()
		{
			mLock.WriterUnlock();
		}

	private:
		RWLock& mLock;
	};
}