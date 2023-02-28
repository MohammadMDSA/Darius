#pragma once

#include "JobCommon.hpp"

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


    void AssignTasks(std::vector<Task> const& tasks, OnFinishCallback const& callback = nullptr);

    void AssignTask(Task const& task, OnFinishCallback const& callback = nullptr);

    void AssignTaskToAllThreads(Task const& task);

    NODISCARD uint32_t GetNumberOfAvailableThreads();

    void WaitForThreadsToFinish();

    bool IsMainThread();
}