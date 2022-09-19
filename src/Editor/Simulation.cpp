#include "pch.hpp"
#include "Simulation.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Renderer/GraphicsUtils/Profiling/Profiling.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

namespace Darius::Editor::Simulate
{

	bool							_Initialized = false;

	bool							Running = false;
	bool							Paused = false;
	bool							Stepping = false;

	D_TIME::StepTimer*				Timer;

	D_SERIALIZATION::Json			SceneDump;

	void Initialize()
	{
		D_ASSERT(!_Initialized);

		D_WORLD::Initialize();
		Timer = D_TIME::GetStepTimer();

		_Initialized = true;

	}

	void Shutdown()
	{
		D_ASSERT(_Initialized);
		D_WORLD::Shutdown();
	}

	void INLINE PauseTime()
	{
		Timer->Pause();
		D_PROFILING::Pause();
	}

	void INLINE ResumeTime()
	{
		Timer->Resume();
		D_PROFILING::Resume();
	}

	void Update()
	{
		
		Timer->Tick([]() {
			D_WORLD::Update(Timer->GetElapsedSeconds());
			});

		D_WORLD::UpdateObjectsConstatns();

		if (Stepping)
		{
			PauseTime();
			Stepping = false;
		}

	}

	void Run()
	{
		if (Running)
			return;
		
		// Saving a dump of scene to be able to reload after simulation stop
		D_WORLD::DumpScene(SceneDump);

		ResumeTime();

		Running = true;
		Paused = false;
	}

	void Stop()
	{
		if (!Running)
			return;

		PauseTime();

		// Resetting scene to what it originally was
		D_WORLD::LoadSceneDump(SceneDump);

		Running = false;
		Paused = false;
	}

	void Pause()
	{
		if (!Running || Paused)
			return;

		PauseTime();

		Paused = true;
	}

	void Resume()
	{
		if (!Running || !Paused)
			return;

		ResumeTime();

		Paused = false;
	}

	void Step()
	{
		if (!Paused || Stepping)
			return;

		ResumeTime();

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
