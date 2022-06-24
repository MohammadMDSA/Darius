#pragma once

#include "JobCommon.hpp"
#include <Utils/Common.hpp>

namespace Darius::Job
{
    void Init();
    void Shutdown();

    void AssignTasks(std::vector<Task> const& tasks, OnFinishCallback const& callback = nullptr);

    void AssignTask(Task const& task, OnFinishCallback const& callback = nullptr);

    void AssignTaskToAllThreads(Task const& task);

    NODISCARD uint32_t GetNumberOfAvailableThreads();

    void WaitForThreadsToFinish();

    bool IsMainThread();
}