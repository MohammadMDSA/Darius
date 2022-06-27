#pragma once
#include <functional>
#include <Core/Containers/ConcurrentQueue.hpp>

namespace Darius::Job
{
    using ThreadNumber = uint32_t;
    using Task = std::function<void(ThreadNumber threadNumber, ThreadNumber totalThreadCount)>;
    using OnFinishCallback = std::function<void()>;

    template<typename T>
    using ThreadSafeQueue = Darius::Core::Containers::ConcurrentQueue<T>;
}