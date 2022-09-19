#include "pch.hpp"
#include "Simulation.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

namespace Darius::Editor::Simulate
{

	bool							_Initialized = false;

	bool							Running = false;
	bool							Paused = false;
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
		if (Running && (!Paused || Stepping))
			D_WORLD::Update(elapsedTime);

		D_WORLD::UpdateObjectsConstatns();

		if (Stepping)
			Stepping = false;

	}

	void Run()
	{
		if (Running)
			return;

		Running = true;
		Paused = false;
	}

	void Stop()
	{
		if (!Running)
			return;

		Running = false;
		Paused = false;
	}

	void Pause()
	{
		if (!Running || Paused)
			return;

		Paused = true;
	}

	void Resume()
	{
		if (!Running || !Paused)
			return;
		Paused = false;
	}

	void Step()
	{
		if (!Paused || Stepping)
			return;

		Stepping = true;
	}

	bool IsSimulating()
	{
		return Running;
	}

	bool IsPaused()
	{
		return Paused;
	}

	bool IsStepping()
	{
		return Stepping;
	}
}
