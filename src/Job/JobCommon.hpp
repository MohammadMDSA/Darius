#pragma once
#include <functional>
#include <Core/Containers/ConcurrentQueue.hpp>

#include <Libs/enkiTS/src/TaskScheduler.h>

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
}