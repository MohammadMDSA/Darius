#pragma once

#include "JobCommon.hpp"
#include <Utils/Common.hpp>

#ifndef D_JOB
#define D_JOB Darius::Job
#endif // !D_JOB


namespace Darius::Job
{
    void Initialize();
    void Shutdown();

    void AssignTasks(std::vector<Task> const& tasks, OnFinishCallback const& callback = nullptr);

    void AssignTask(Task const& task, OnFinishCallback const& callback = nullptr);

    void AssignTaskToAllThreads(Task const& task);

    NODISCARD uint32_t GetNumberOfAvailableThreads();

    void WaitForThreadsToFinish();

    bool IsMainThread();
}