#include "Job.hpp"

#include <Core/Memory/Memory.hpp>
#include <Core/Containers/Map.hpp>
#include <Utils/Assert.hpp>

#include <thread>

namespace Darius::Job
{
	bool										_initialized = false;
	enki::TaskScheduler							Scheduler;

	D_CONTAINERS::DUnorderedMap<ThreadType, ThreadNumber> ThreadTypeMapping;

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);

		// We create more threads than the hardware can run,
		// because the IO thread will spend most of it's time idle or blocked
		// and therefore not scheduled for CPU time by the OS
		enki::TaskSchedulerConfig config;
		config.numTaskThreadsToCreate += 1;

		ThreadTypeMapping[ThreadType::FileIO] = config.numTaskThreadsToCreate;

		Scheduler.Initialize(config);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
		Scheduler.WaitforAllAndShutdown();
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
	}

	void AddPinnedTaskAndWait(PinnedTaskFunction func, ThreadType thread)
	{
		enki::LambdaPinnedTask task(ThreadTypeMapping.at(thread), func);
		Scheduler.AddPinnedTask(&task);
		Scheduler.WaitforTask(&task);
	}

}