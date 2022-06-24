#include "ThreadPool.hpp"
#include <Utils/Assert.hpp>
#include <Utils/Log.hpp>

namespace Darius::Job
{
	ThreadPool::ThreadPool()
	{
		m_mainThreadId = std::this_thread::get_id();
		m_numberOfThreads = std::thread::hardware_concurrency();
		if (m_numberOfThreads < 2)
		{
			m_isAlive = false;
		}
		else
		{
			m_isAlive = true;
			for (ThreadNumber threadIndex = 0; threadIndex < m_numberOfThreads; m_numberOfThreads++)
			{
				m_threadObjects.emplace_back(std::make_unique<ThreadObject>(threadIndex, *this));
			}
		}
	}

	bool ThreadPool::IsMainThread() const
	{
		return std::this_thread::get_id() == m_mainThreadId;
	}

	ThreadPool::~ThreadPool()
	{
		m_isAlive = false;
		for (auto const& thread : m_threadObjects)
			thread->Notify();
		for (auto const& thread : m_threadObjects)
			thread->Join();
	}

	void ThreadPool::AssignTask(Task const& task) const
	{
		D_ASSERT(task != nullptr);

		if (m_isAlive)
		{
			ThreadObject* freeThread = GetFreeThread();

			D_ASSERT(freeThread != nullptr);
			freeThread->Assign(task);
		}
		else
		{
			task(0, 1);
		}
	}

	ThreadPool::ThreadObject* ThreadPool::GetFreeThread() const
	{
		if (m_isAlive)
		{
			ThreadObject* freeThread = nullptr;
			size_t freeThreadTaskCount = 0;
			for (auto& thread : m_threadObjects)
			{
				if (freeThread == nullptr || freeThreadTaskCount > thread->InQueueTasksCount())
				{
					freeThread = thread.get();
					freeThreadTaskCount = thread->InQueueTasksCount();
				}
			}
			return freeThread;
		}
		return nullptr;
	}

	void ThreadPool::AssignTaskToAllThreads(Task const& task) const
	{
		D_ASSERT(task != nullptr);

		if (m_isAlive)
		{
			for (auto& thread : m_threadObjects)
			{
				thread->Assign(task);
			}
		}
		else
		{
			task(0, 1);
		}
	}

	ThreadPool::ThreadObject::ThreadObject(ThreadNumber const threadNumber, ThreadPool& parent, size_t maxTaskCount) :
		m_parent(parent),
		m_threadNumber(threadNumber),
		m_tasks(maxTaskCount),
		m_taskCount(0)
	{
		D_ASSERT_M(maxTaskCount & (maxTaskCount - 1) == 0, L"Max task count must be power of two");
		m_thread = std::make_unique<std::thread>([this]()-> void
		{
			MainLoop();
		});
	}

	auto ThreadPool::ThreadObject::Assign(Task const& task) -> void
	{
		while (!m_tasks.bounded_push(task))
		{
			m_condition.notify_one();
		}
		m_taskCount++;
	}

	ThreadNumber ThreadPool::GetNumberOfAvailableThreads() const
	{
		return m_numberOfThreads;
	}

	// Alive while either has work to do or has dead parent
	bool ThreadPool::ThreadObject::AwakeCondition()
	{
		return !m_tasks.empty() || !m_parent.m_isAlive;
	}

	void ThreadPool::ThreadObject::Join() const
	{
		m_thread->join();
	}

	bool ThreadPool::ThreadObject::IsFree()
	{
		auto const isFree = m_tasks.empty() && !m_isBusy;
		if (!isFree)
		{
			m_condition.notify_one();
		}

		return isFree;
	}

	size_t ThreadPool::ThreadObject::InQueueTasksCount()
	{
		return m_taskCount;
	}

	void ThreadPool::ThreadObject::Notify()
	{
		m_condition.notify_one();
	}

	ThreadNumber ThreadPool::ThreadObject::GetThreadNumber() const
	{
		return m_threadNumber;
	}

	void ThreadPool::ThreadObject::MainLoop()
	{
		std::mutex mutex{};
		std::unique_lock<std::mutex> mLock{ mutex };
		while (m_parent.m_isAlive)
		{
			m_condition.wait(mLock, [this]()-> bool
			{
				return AwakeCondition();
			});

			m_isBusy = true;
			while (!m_tasks.empty() && m_parent.m_isAlive)
			{
				Task currentTask;
				while (!m_tasks.pop(currentTask));

				try
				{
					if (currentTask != nullptr)
					{
						currentTask(m_threadNumber, m_parent.m_numberOfThreads);
					}
				}
				catch (const std::exception& ex)
				{
					while (!m_parent.m_exceptions.bounded_push(ex.what()));
				}
				m_taskCount--;
			}
			m_isBusy = false;
		}
	}

	bool ThreadPool::MainThreadAwakeCondition() const
	{
		bool shouldAwake = true;
		for (auto const& threadObject : m_threadObjects)
		{
			if (!threadObject->IsFree())
			{
				threadObject->Notify();
				shouldAwake = false;
			}
		}
		return shouldAwake;
	}

	void ThreadPool::WaitForThreadsToFinish()
	{
		D_ASSERT(std::this_thread::get_id() == m_mainThreadId);
		while (!MainThreadAwakeCondition())
		{
			for (auto const& threadObject : m_threadObjects)
			{
				threadObject->Notify();
			}
			std::this_thread::yield();
		}

		D_ASSERT(AllThreadsAreIdle());

		while (!m_exceptions.empty())
		{
			std::string ex;
			while (!m_exceptions.pop(ex));

			D_LOG_ERROR(ex);
		}
	}

	bool ThreadPool::AllThreadsAreIdle()
	{
		for (auto const& threadObject : m_threadObjects)
		{
			if (!threadObject->IsFree())
				return false;
		}
		return true;
	}
}