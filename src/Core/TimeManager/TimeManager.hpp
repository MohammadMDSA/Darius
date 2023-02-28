#pragma once

#include "StepTimer.hpp"

#include <Core/Serialization/Json.hpp>

#ifndef D_TIME
#define D_TIME Darius::Core::TimeManager
#endif // !D_TIME_MANAGER

using namespace Microsoft::WRL;

namespace Darius::Core::TimeManager
{
	void Initialize(D_SERIALIZATION::Json const& settings);
	void Shutdown();
#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

	void EnableFixedTimeStep(double targetElapsedSeconds);
	void DisableFixedTimeStep();
	float GetTargetElapsedSeconds();
	float GetDeltaTime();
	float GetTotalTime();
	uint32_t GetFrameCount();

	StepTimer* GetStepTimer();
}
