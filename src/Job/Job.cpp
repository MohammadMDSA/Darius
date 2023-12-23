#include "Job.hpp"

#include <Core/Memory/Memory.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/Containers/Set.hpp>
#include <Utils/Assert.hpp>

#include <thread>

namespace Darius::Job
{
	bool										_initialized = false;
	enki::TaskScheduler							Scheduler;

	D_CONTAINERS::DConcurrentSet<IPinnedTask*>				PinnedTasks;

	D_CONTAINERS::DUnorderedMap<ThreadType, ThreadNumber>	ThreadTypeMapping;
	D_CONTAINERS::DUnorderedMap<ThreadType, IPinnedTask*>	PinnedTaskRunners;


	struct RunFileIOPinnedTaskLoopTask : IPinnedTask {

		void Execute() override {
			while (TaskScheduler->GetIsRunning() && Executing && !TaskScheduler->GetIsShutdownRequested()) {
				TaskScheduler->WaitForNewPinnedTasks(); // this thread will 'sleep' until there are new pinned tasks
				TaskScheduler->RunPinnedTasks();
			}
		}

		enki::TaskScheduler*	TaskScheduler;
		bool                    Executing = true;
	};

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);

		// We create more threads than the hardware can run,
		// because the IO thread will spend most of it's time idle or blocked
		// and therefore not scheduled for CPU time by the OS
		enki::TaskSchedulerConfig config;

		Scheduler.Initialize(config);


		// Setting up FileIO Task Runner
		{
			ThreadTypeMapping[ThreadType::FileIO] = Scheduler.GetNumTaskThreads() - 1;

			auto fileIORunner = new RunFileIOPinnedTaskLoopTask;
			PinnedTaskRunners.emplace(ThreadType::FileIO, fileIORunner);
			fileIORunner->threadNum = ThreadTypeMapping[ThreadType::FileIO];
			fileIORunner->TaskScheduler = &Scheduler;
			Scheduler.AddPinnedTask(fileIORunner);
		}

		_initialized = true;

	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
		Scheduler.ShutdownNow();

		// Delete threads
		for (auto [_, taskRunner] : PinnedTaskRunners)
			delete taskRunner;
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	bool IsMainThread()
	{
		D_ASSERT(_initialized);
		return GetThreadNum() == 0;
	}

	ThreadNumber GetThreadNum()
	{
		return Scheduler.GetThreadNum();
	}

	void WaitForAll()
	{
		Scheduler.WaitforAll();
	}

	void WaitForTask(ICompletable* task, TaskPriority priority)
	{
		Scheduler.WaitforTask(task, priority);
	}

	uint32_t  GetNumTaskThreads()
	{
		return Scheduler.GetNumTaskThreads();
	}

	void RunPinnedTasks()
	{
		Scheduler.RunPinnedTasks();
	}

	void AddTaskSet(ITaskSet* task)
	{
		Scheduler.AddTaskSetToPipe(task);
	}

	void AddTaskSetAndWait(uint32_t setSize, TaskSetFunction func, uint32_t minRangeSize)
	{
		enki::TaskSet task(setSize, func);
		task.m_MinRange = minRangeSize;
		Scheduler.AddTaskSetToPipe(&task);
		Scheduler.WaitforTask(&task);
	}

	void AddTaskSetAndWait(TaskSetFunction func)
	{
		enki::TaskSet task(func);
		Scheduler.AddTaskSetToPipe(&task);
		Scheduler.WaitforTask(&task);
	}

	void AddTaskSetAndWait(D_CONTAINERS::DVector<std::function<void()>> funcs)
	{
		AddTaskSetAndWait((uint32_t)funcs.size(), [funcs = funcs](TaskPartition range, int thredNum)
			{
				for (uint64_t i = range.start; i < range.end; ++i)
				{
					funcs[i]();
				}
			});
	}


	void AddPinnedTask(IPinnedTask* task, ThreadType thread)
	{
		task->threadNum = ThreadTypeMapping.at(thread);
		Scheduler.AddPinnedTask(task);
		PinnedTasks.insert(task);

		if (IsMainThread())
		{
			D_CONTAINERS::DVector<IPinnedTask*> toBeRemoved;
			toBeRemoved.reserve(PinnedTasks.size());

			for (IPinnedTask* task : PinnedTasks)
			{
				if (task->GetIsComplete())
				{
					toBeRemoved.push_back(task);
				}
			}

			for (IPinnedTask* task : toBeRemoved)
			{
				PinnedTasks.unsafe_erase(task);
				delete task;
			}

		}
	}

	void AddPinnedTaskAndWait(PinnedTaskFunction func, ThreadType thread)
	{
		enki::LambdaPinnedTask task(ThreadTypeMapping.at(thread), func);
		Scheduler.AddPinnedTask(&task);
		Scheduler.WaitforTask(&task);
	}

}