#include "Job.hpp"
#include "ThreadPool.hpp"
#include <Utils/Assert.hpp>
#include <Core/Memory/Memory.hpp>

namespace Darius::Job
{
	struct State
	{
		ThreadPool threadPool{};
	};

	State* state = nullptr;

	class TaskTracker
	{
	public:

		explicit TaskTracker(size_t const totalTaskCount, OnFinishCallback callback) :
			m_finishCallBack(callback),
			m_remainingTasksCount(static_cast<int>(totalTaskCount))
		{
			D_ASSERT(m_remainingTasksCount > 0);
		}

		~TaskTracker() = default;

		TaskTracker(TaskTracker const&) noexcept = delete;
		TaskTracker(TaskTracker&&) noexcept = delete;
		TaskTracker& operator = (TaskTracker const&) noexcept = delete;
		TaskTracker& operator = (TaskTracker&&) noexcept = delete;

		void OnTaskComplete()
		{
			m_remainingTasksCount--;
			D_ASSERT(m_remainingTasksCount >= 0);
			if (m_remainingTasksCount == 0 && m_finishCallBack != nullptr)
			{
				m_finishCallBack();
			}
		}

	private:
		OnFinishCallback		m_finishCallBack;
		std::atomic<int>		m_remainingTasksCount;
	};

	void Initialize()
	{
		D_ASSERT(state == nullptr);
		state = new State();
	}

	void Shutdown()
	{
		D_ASSERT(state != nullptr);
		D_free(state);
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	void AssignTasks(std::vector<Task> const& tasks, OnFinishCallback const& callback)
	{
		D_ASSERT(!tasks.empty());

		auto const tasksCount = tasks.size();

		std::shared_ptr<TaskTracker> tracker = std::make_shared<TaskTracker>(tasksCount, callback);

		for (auto& task : tasks)
		{
			D_ASSERT(task != nullptr);

			state->threadPool.AssignTask([tracker, task](ThreadNumber const threadNumber, ThreadNumber const threadCount)->void
				{
					D_ASSERT(task != nullptr);
					task(threadNumber, threadCount);
					D_ASSERT(tracker != nullptr);
					tracker->OnTaskComplete();
				});
		}
	}

	void AssignTask(Task const& task, OnFinishCallback const& callback)
	{
		D_ASSERT(task != nullptr);
		AssignTasks(std::vector<Task>{task}, callback);
	}

	void AssignTaskToAllThreads(Task const& task)
	{
		D_ASSERT(task != nullptr);
		state->threadPool.AssignTaskToAllThreads(task);
	}

	uint32_t GetNumberOfAvailableThreads()
	{
		return state->threadPool.GetNumberOfAvailableThreads();
	}

	void WaitForThreadsToFinish()
	{
		state->threadPool.WaitForThreadsToFinish();
	}

	bool IsMainThread()
	{
		D_ASSERT(state != nullptr);
		return state->threadPool.IsMainThread();
	}
}