#pragma once
#include <functional>

#include <Utils/Common.hpp>

#include <Libs/enkiTS/src/TaskScheduler.h>

#include <atomic>

#ifndef D_JOB
#define D_JOB Darius::Job
#endif // !D_JOB

namespace Darius::Job
{
    enum class ThreadType : uint16_t
    {
        FileIO
    };

    typedef enki::TaskSetPartition          TaskPartition;
    typedef uint32_t                        ThreadNumber;

    typedef enki::ICompletable              ICompletable;
    typedef enki::ITaskSet                  ITaskSet;
    typedef enki::IPinnedTask               IPinnedTask;

    typedef std::function<void(TaskPartition range, ThreadNumber threadNumber)> TaskSetFunction;
    typedef std::function<void()>           PinnedTaskFunction;
    typedef std::function<void()>           OnFinishCallback;

    typedef enki::TaskPriority              TaskPriority;

    class CancellationToken
    {
    public:
        INLINE void         SetCancelled() { mCancelled.store(true); }
        INLINE bool         IsCancelled() const { return mCancelled.load(); }

    private:
        std::atomic_bool    mCancelled = { false };
    };

}