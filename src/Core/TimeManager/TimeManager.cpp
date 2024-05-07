#include "Core/pch.hpp"
#include "TimeManager.hpp"
#include "SystemTime.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Core::TimeManager
{
	std::unique_ptr<StepTimer> Timer = nullptr;


	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(Timer == nullptr);
		Timer = std::make_unique<StepTimer>();

		D_TIME::SystemTime::Initialize();
	}

	void Shutdown()
	{
		D_ASSERT(Timer);

		Timer.reset();
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	void EnableFixedTimeStep(double targetElapsedSeconds)
	{
		Timer->SetFixedTimeStep(true);
		Timer->SetTargetElapsedSeconds(targetElapsedSeconds);
	}

	void DisableFixedTimeStep()
	{
		Timer->SetFixedTimeStep(false);
	}

	float GetTargetElapsedSeconds()
	{
		return (float)Timer->GetTargetElapsedSeconds();
	}

	StepTimer* GetStepTimer()
	{
		return Timer.get();
	}

	float GetDeltaTime()
	{
		return (float)Timer->GetElapsedSeconds();
	}

	float GetTotalTime()
	{
		return (float)Timer->GetTotalSeconds();
	}

	uint32_t GetFrameCount()
	{
		return Timer->GetFrameCount();
	}
}