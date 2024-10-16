#include "Engine/pch.hpp"
#include "SubsystemRegistry.hpp"
#include "EngineContext.hpp"

#include <Animation/AnimationManager.hpp>
#include <Audio/AudioManager.hpp>
#include <Core/Containers/List.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/Input.hpp>
#include <Core/Serialization/TypeSerializer.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <FBX/FBXSystem.hpp>
#include <Scene/Scene.hpp>
#include <Job/Job.hpp>
#include <Physics/PhysicsManager.hpp>
#include <Renderer/RendererManager.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Utils/Assert.hpp>

#include <functional>

#ifdef _D_EDITOR
#define REGISTER_SUBSYSTEM(name,subsys) \
{ \
	SubsystemContainer[name] = { \
		subsys::OptionsDrawer \
	}; \
}
#else
#define REGISTER_SUBSYSTEM(name,subsys) \
{ \
	_subsystemContainer[name] = { \
	}; \
}
#endif

using namespace std;

namespace Darius::Subsystems
{

	struct SubsystemFuncCollection
	{
		/*D_CONTAINERS::DList<function<void()>>						initializers;
		D_CONTAINERS::DList<function<void()>>						shutdowners;*/
		function<bool(D_SERIALIZATION::Json&)>						OptionsDrawer;
	};

	bool														_initialized = false;
	D_CONTAINERS::DUnorderedMap<std::string, SubsystemFuncCollection> SubsystemContainer;

	void RegisterSubsystems()
	{
		REGISTER_SUBSYSTEM("Animation", D_ANIMATION);
		REGISTER_SUBSYSTEM("Audio", D_AUDIO);
		REGISTER_SUBSYSTEM("Physics", D_PHYSICS);
#ifdef _D_EDITOR
		REGISTER_SUBSYSTEM("Debug Draw", D_DEBUG_DRAW);
#endif // _D_EDITOR
		REGISTER_SUBSYSTEM("Input", D_INPUT);
		REGISTER_SUBSYSTEM("Time", D_TIME);
		REGISTER_SUBSYSTEM("Job", D_JOB);
		REGISTER_SUBSYSTEM("Renderer", D_RENDERER);
		REGISTER_SUBSYSTEM("Graphics", D_GRAPHICS);
		REGISTER_SUBSYSTEM("Resource Manager", D_RESOURCE);
	}

	void InitialzieSubsystems(HWND window, int width, int height, D_FILE::Path const& projectPath)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		D_SERIALIZATION::Json settings;

		D_FILE::ReadJsonFile(D_ENGINE_CONTEXT::GetEngineSettingsPath(), settings);


		/*for (auto initializer : initializers)
		{
			initializer();
		}*/

		D_SERIALIZATION::Initialize();

		// Initializing the resource manager
		D_RESOURCE::Initialize(settings["Resource Manager"]);

		D_GRAPHICS::Initialize(window, width, height, settings["Graphics"]);

		D_RENDERER::Initialize(settings["Renderer"]);

		// Creating device and window resources
		/*CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();*/

		D_WORLD::Initialize();

		D_JOB::Initialize(settings["Job"]);

		// Initialing the tiem manager
		D_TIME::Initialize(settings["Time"]);

		// Initializing the input manater
		D_INPUT::Initialize(window, settings["Input"]);

		// Initialize Debug Drawer
#ifdef _D_EDITOR
		D_DEBUG_DRAW::Initialize(settings["Debug Draw"]);
#endif // _D_EDITOR

		// Initializeing physics
		D_PHYSICS::Initialize(settings["Physics"]);

		// Initializing animation
		D_ANIMATION::Initialize(settings["Animation"]);

		// Initializing audio
		D_AUDIO::Initialize(settings["Audio"]);

		// Initializing fbx
		D_FBX::Initialize(settings["Fbx"]);
	}

	void ShutdownSubsystems()
	{
		D_ASSERT(_initialized);

		/*for (auto initializer : initializers)
		{
			initializer();
		}*/
		D_FBX::Shutdown();
		D_AUDIO::Shutdown();
		D_ANIMATION::Shutdown();
		D_PHYSICS::Shutdown();
#ifdef _D_EDITOR
		D_DEBUG_DRAW::Shutdown();
#endif // _D_EDITOR
		D_INPUT::Shutdown();
		D_TIME::Shutdown();
		D_JOB::Shutdown();
		D_WORLD::Shutdown();
		D_RENDERER::Shutdown();
		D_GRAPHICS::Shutdown();
		D_RESOURCE::Shutdown();
		D_SERIALIZATION::Shutdown();
	}

#ifdef _D_EDITOR
	void GetSubsystemsOptionsDrawer(D_CONTAINERS::DUnorderedMap<std::string, std::function<bool(D_SERIALIZATION::Json&)>>& systemOption)
	{

		systemOption.clear();

		for (auto keyVal : SubsystemContainer)
		{
			systemOption[keyVal.first] = keyVal.second.OptionsDrawer;
		}
	}
#endif
}
