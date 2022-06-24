#pragma once

#include "JobCommon.hpp"
#include <Utils/Common.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace Darius::Job
{
    class ThreadPool
    {
    public:

        explicit ThreadPool();
        // We can have a threadPool with custom number of threads

        ~ThreadPool();

        ThreadPool(ThreadPool const&) noexcept = delete;
        ThreadPool(ThreadPool&&) noexcept = delete;
        ThreadPool& operator = (ThreadPool const&) noexcept = delete;
        ThreadPool& operator = (ThreadPool&&) noexcept = delete;

        bool IsMainThread() const;

        void AssignTask(Task const& task) const;

        void AssignTaskToAllThreads(Task const& task) const;

        NODISCARD ThreadNumber GetNumberOfAvailableThreads() const;

        void WaitForThreadsToFinish();

        class ThreadObject
        {
        public:

            explicit ThreadObject(ThreadNumber threadNumber, ThreadPool& parent, size_t maxTaskCount);

            ~ThreadObject() = default;

            ThreadObject(ThreadObject const&) noexcept = delete;
            ThreadObject(ThreadObject&&) noexcept = delete;
            ThreadObject& operator = (ThreadObject const&) noexcept = delete;
            ThreadObject& operator = (ThreadObject&&) noexcept = delete;

            void Assign(Task const& task);

            void Join() const;

            NODISCARD bool IsFree();

            NODISCARD size_t InQueueTasksCount();

            void Notify();

            NODISCARD ThreadNumber GetThreadNumber() const;

            bool AwakeCondition();

        private:

            void MainLoop();

            ThreadPool&                     m_parent;
            ThreadNumber                    m_threadNumber;
            ThreadSafeQueue<Task>           m_tasks;
            std::condition_variable         m_condition;
            std::unique_ptr<std::thread>    m_thread;
            std::atomic<bool>               m_isBusy = false;
            size_t                          m_taskCount;
        };

        bool AllThreadsAreIdle();

    private:

        bool MainThreadAwakeCondition() const;
        ThreadObject* GetFreeThread() const;

        std::vector<std::unique_ptr<ThreadObject>>  m_threadObjects;

        bool                                        m_isAlive = true;

        ThreadNumber                                m_numberOfThreads = 0;

        ThreadSafeQueue<std::string>               m_exceptions{};

        std::thread::id                             m_mainThreadId{};

    };
}