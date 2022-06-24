#pragma once
#include <functional>
#include <boost/lockfree/queue.hpp>

namespace Darius::Job
{
    using ThreadNumber = uint32_t;
    using Task = std::function<void(ThreadNumber threadNumber, ThreadNumber totalThreadCount)>;
    using OnFinishCallback = std::function<void()>;

    template<typename T>
    using ThreadSafeQueue = boost::lockfree::queue<T>;
}