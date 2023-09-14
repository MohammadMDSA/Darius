#pragma once

#include "JobCommon.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>

#ifndef D_JOB
#define D_JOB Darius::Job
#endif // !D_JOB


namespace Darius::Job
{
    void Initialize(D_SERIALIZATION::Json const& settings);
    void Shutdown();
#ifdef _D_EDITOR
    bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

    // Thread 0 is main thread, otherwise use threadNum
    // Pinned tasks can be added from any thread
    void                AddPinnedTaskAndWait(PinnedTaskFunction func, ThreadType thread);

    // Thread 0 is main thread, otherwise use threadNum
    // Pinned tasks can be added from any thread
    void                AddPinnedTask(IPinnedTask* task, ThreadType thread);

    // Adds the TaskSet to pipe and returns if the pipe is not full.
    // If the pipe is full, pTaskSet is run.
    // should only be called from main thread, or within a task
    void                AddTaskSetAndWait(uint32_t setSize, TaskSetFunction func, uint32_t minRangeSize = 1u);
    void                AddTaskSetAndWait(TaskSetFunction func);
    void                AddTaskSetAndWait(D_CONTAINERS::DVector<std::function<void()>> funcs);

    // Adds the TaskSet to pipe and returns if the pipe is not full.
    // If the pipe is full, pTaskSet is run.
    // should only be called from main thread, or within a task
    void                AddTaskSet(ITaskSet* task);

    // This function will run any IPinnedTask* for current thread, but not run other
    // Main thread should call this or use a wait to ensure it's tasks are run.
    void                RunPinnedTasks();

    NODISCARD uint32_t  GetNumTaskThreads();

    // Waits for all task sets to complete - not guaranteed to work unless we know we
    // are in a situation where tasks aren't being continuously added.
    // If you are running tasks which loop, make sure to check GetIsWaitforAllCalled() and exit
    void                WaitForAll();

    void                WaitForTask(ICompletable* task, TaskPriority priority = TaskPriority(TaskPriority::TASK_PRIORITY_NUM - 1));

    NODISCARD bool      IsMainThread();

    // Returns the current task threadNum
    // Will return 0 for thread which initialized the task scheduler,
    // and all other non-enkiTS threads which have not been registered ( see RegisterExternalTaskThread() ),
    // and < GetNumTaskThreads() for all threads.
    // It is guaranteed that GetThreadNum() < GetNumTaskThreads()
    NODISCARD ThreadNumber GetThreadNum();
}