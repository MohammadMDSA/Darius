#include "Core/pch.hpp"
#include "TimeManager.hpp"
#include "SystemTime.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Core::TimeManager
{
	std::unique_ptr<StepTimer> Timer = nullptr;


	void Initialize()
	{
		D_ASSERT(Timer == nullptr);
		Timer = std::make_unique<StepTimer>();

		D_TIME::SystemTime::Initialize();
	}

	void Shutdown()
	{
		D_ASSERT(Timer);

		Timer.release();
	}

	void EnableFixedTimeStep(double targetElapsedSeconds)
	{
		Timer->SetFixedTimeStep(true);
		Timer->SetTargetElapsedSeconds(targetElapsedSeconds);
	}

	void DisableFixedTimeStep()
	{
		Timer->SetFixedTimeStep(false);
	}

	StepTimer* GetStepTimer()
	{
		return Timer.get();
	}
}