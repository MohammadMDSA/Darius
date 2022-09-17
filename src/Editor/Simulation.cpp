#include "pch.hpp"
#include "Simulation.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

#include <boost/timer/timer.hpp>

namespace Darius::Editor::Simulate
{

	bool							_Initialized = false;

	bool							IsRunning = false;
	bool							Stepping = false;

	void Initialize()
	{
		D_ASSERT(!_Initialized);

		D_WORLD::Initialize();

		_Initialized = true;

	}

	void Shutdown()
	{
		D_ASSERT(_Initialized);
		D_WORLD::Shutdown();
	}

	void Update(float elapsedTime)
	{
		if (IsRunning || Stepping)
			D_WORLD::Update(elapsedTime);


		if (Stepping)
			Stepping = false;

	}

	void Run()
	{
		if (IsRunning)
			return;

		IsRunning = true;
		boost::timer::cpu_timer
	}

	void Pause()
	{
		if (!IsRunning)
			return;

		IsRunning = false;
	}

	void Step()
	{
		if (IsRunning || Stepping)
			return;

		Stepping = true;
	}

	bool IsSimulating()
	{
		return IsRunning;
	}
}
