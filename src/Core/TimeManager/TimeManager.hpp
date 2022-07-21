#pragma once

#include "StepTimer.hpp"

#ifndef D_TIME
#define D_TIME Darius::Core::TimeManager
#endif // !D_TIME_MANAGER

using namespace Microsoft::WRL;

namespace Darius::Core::TimeManager
{
	void Initialize();
	void Shutdown();

	void EnableFixedTimeStep(double targetElapsedSeconds);
	void DisableFixedTimeStep();

	StepTimer* GetStepTimer();
}
