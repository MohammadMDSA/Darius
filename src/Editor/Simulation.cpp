#include "pch.hpp"
#include "Simulation.hpp"
#include "EditorContext.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Signal.hpp>
#include <Debug/DebugDraw.hpp>
#include <Physics/PhysicsManager.hpp>
#include <Renderer/GraphicsUtils/Profiling/Profiling.hpp>
#include <Renderer/GraphicsUtils/Profiling/Profiling.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

namespace Darius::Editor::Simulate
{

	bool							_Initialized = false;

	bool							Running = false;
	bool							Paused = false;
	bool							Stepping = false;

	D_CORE::Signal<void()>			RunSignal;
	D_CORE::Signal<void()>			StopSignal;

	D_TIME::StepTimer* Timer;

	D_SERIALIZATION::Json			SceneDump;

	void Initialize()
	{
		D_ASSERT(!_Initialized);

		D_WORLD::Initialize();
		Timer = D_TIME::GetStepTimer();
		Timer->SetFixedTimeStep(false);

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

		D_DEBUG_DRAW::Clear();
		{
			D_PROFILING::ScopedTimer simProfiler(L"Update Simulation");
			Timer->Tick([]() {

				// Clearing debug draws

				D_WORLD::Update(Timer->GetElapsedSeconds());

				if (Stepping)
				{
					PauseTime();
					Stepping = false;
				}
				});

			{
				D_PROFILING::ScopedTimer simPhysProf(L"Update Physics");
				D_PHYSICS::Update(!Timer->IsPaused());
				D_DEBUG_DRAW::FinalizeUpload();
			}
		}

		{
			D_PROFILING::ScopedTimer objConstProfiling(L"Update Object Constants");
			D_WORLD::UpdateObjectsConstatns();
		}


	}

	void Run()
	{
		if (Running)
			return;

		// Saving a dump of scene to be able to reload after simulation stop
		SceneDump.clear();
		D_WORLD::DumpScene(SceneDump);

		ResumeTime();

		Running = true;
		Paused = false;

		D_WORLD::SetAwake();

		RunSignal();
	}

	void Stop()
	{
		if (!Running)
			return;

		PauseTime();
		Timer->Reset();

		// Resetting scene to what it originally was
		D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
		D_WORLD::ClearScene();
		D_WORLD::LoadSceneDump(SceneDump);

		Running = false;
		Paused = false;

		StopSignal();
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

	void SubscribeOnRun(std::function<void()> callback)
	{
		RunSignal.connect(callback);
	}

	void SubscribeOnStop(std::function<void()> callback)
	{
		StopSignal.connect(callback);
	}
}
