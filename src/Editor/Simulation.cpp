#include "pch.hpp"

#include "Simulation.hpp"
#include "EditorContext.hpp"

#include <Audio/AudioManager.hpp>
#include <Animation/AnimationManager.hpp>
#include <Core/Input.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Signal.hpp>
#include <Debug/DebugDraw.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Physics/PhysicsManager.hpp>
#include <Renderer/RendererManager.hpp>
#include <Scene/Scene.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
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

		Timer = D_TIME::GetStepTimer();
		Timer->SetFixedTimeStep(false);

		_Initialized = true;

	}

	void Shutdown()
	{
		D_ASSERT(_Initialized);
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

	void Update(float editorDeltaTime, std::function<void()> externalContextUpdate)
	{

		D_DEBUG_DRAW::Clear();
		{
			D_PROFILING::ScopedTimer simProfiler(L"Update Simulation");

			static float lastElapse = 0.f;

			if(Timer->IsPaused())
			{
				{
					D_PROFILING::ScopedTimer inputProfiling(L"Update Input");
					D_INPUT::Update(editorDeltaTime);
				}
				{
					if(externalContextUpdate)
						externalContextUpdate();
				}

				// Physics
				{
					D_PROFILING::ScopedTimer simPhysProf(L"Update Physics");
					D_PHYSICS::Update(false, editorDeltaTime);
				}
			}

			Timer->Tick([&]()
				{
					if(!Timer->IsPaused())
					{
						auto deltaTime = (float)Timer->GetElapsedSeconds();

						{
							D_PROFILING::ScopedTimer inputProfiling(L"Update Input");
							D_INPUT::Update(deltaTime);
						}

						{
							if(externalContextUpdate)
								externalContextUpdate();
						}

						// Physics
						{
							D_PROFILING::ScopedTimer simPhysProf(L"Update Physics");
							D_PHYSICS::Update(true, deltaTime);
						}

						// World Logic
						{
							D_PROFILING::ScopedTimer SceneUpdateProfiling(L"Behaviour Update");
							D_WORLD::Update(deltaTime);
						}
						{
							D_PROFILING::ScopedTimer SceneUpdateProfiling(L"Behaviour Late Update");
							D_WORLD::LateUpdate(deltaTime);
						}

						{
							// Updating animations
							D_PROFILING::ScopedTimer animProf(L"Update Animations");
							D_ANIMATION::Update(deltaTime);
						}

						if(Stepping)
						{
							PauseTime();
							Stepping = false;
						}
					};
				});

		}

		{
			D_PROFILING::ScopedTimer _prof(L"Update Audio");
			D_AUDIO::Update(Timer->IsPaused() ? 0.f : (float)Timer->GetElapsedSeconds());
		}


		// Update GPU and upload stuff
		{
			D_PROFILING::ScopedTimer objConstProfiling(L"Update Renderer");
			D_RENDERER::Update();

			// Call on gizmo
			{
				auto selectedGo = D_EDITOR_CONTEXT::GetSelectedGameObject();
				if(selectedGo)
					selectedGo->OnGizmo();
			}

			D_DEBUG_DRAW::FinalizeUpload();
		}


	}

	void Run()
	{
		if(Running)
			return;

		// Saving a dump of scene to be able to reload after simulation stop
		SceneDump.clear();
		D_WORLD::DumpScene(SceneDump);

		ResumeTime();

		Running = true;
		Paused = false;

		D_WORLD::SetAwake();
		D_DEBUG_DRAW::Clear(true);

		RunSignal();
	}

	void Stop()
	{
		if(!Running)
			return;

		PauseTime();
		Timer->Reset();

		// Resetting scene to what it originally was
		D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
		D_WORLD::ClearScene([]() { D_PHYSICS::Update(true, 0.01f); });
		D_WORLD::LoadSceneDump(SceneDump);
		D_DEBUG_DRAW::Clear(true);

		Running = false;
		Paused = false;

		StopSignal();

	}

	void Pause()
	{
		if(!Running || Paused)
			return;

		PauseTime();

		Paused = true;
	}

	void Resume()
	{
		if(!Running || !Paused)
			return;

		ResumeTime();

		Paused = false;
	}

	void Step()
	{
		if(!Paused || Stepping)
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
