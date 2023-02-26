#include "pch.hpp"
#include "SubsystemRegistry.hpp"

#include <Animation/AnimationManager.hpp>
#include <Core/Input.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Job/Job.hpp>
#include <Physics/PhysicsManager.hpp>
#include <Renderer/GraphicsCore.hpp>
#include <Renderer/Renderer.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>

#include <Core/Containers/List.hpp>
#include <Utils/Assert.hpp>

#include <functional>

using namespace std;

namespace Darius::Subsystems
{
	bool												_initialized = false;
	/*D_CONTAINERS::DList<function<void()>>				initializers;
	D_CONTAINERS::DList<function<void()>>				shutdowners;*/

	void RegisterSubsystems()
	{
		
	}

	void InitialzieSubsystems(HWND window, int width, int height, D_FILE::Path const& projectPath)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		/*for (auto initializer : initializers)
		{
			initializer();
		}*/

		// Initializing the resource manager
		D_RESOURCE::Initialize();

		D_RENDERER_DEVICE::Initialize(window, width, height);
		D_RENDERER::Initialize();

		// Creating device and window resources
		/*CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();*/

		D_JOB::Initialize();

		// Initialing the tiem manager
		D_TIME::Initialize();

		// Initializing the input manater
		D_INPUT::Initialize(window);

		// Initialize Debug Drawer
#ifdef _DEBUG
		D_DEBUG_DRAW::Initialize();
#endif // _DEBUG

		// Initializeing physics
		D_PHYSICS::Initialize();

		// Initializing animation
		D_ANIMATION::Initialize();
	}

	void ShutdownSubsystems()
	{
		D_ASSERT(_initialized);

		/*for (auto initializer : initializers)
		{
			initializer();
		}*/

		D_ANIMATION::Shutdown();
		D_PHYSICS::Shutdown();
#ifdef _DEBUG
		D_DEBUG_DRAW::Shutdown();
#endif // _DEBUG
		D_INPUT::Shutdown();
		D_TIME::Shutdown();
		D_JOB::Shutdown();
		D_RENDERER::Shutdown();
		D_RENDERER_DEVICE::Shutdown();
		D_RESOURCE::Shutdown();
	}

}
