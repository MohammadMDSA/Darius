#include "pch.hpp"
#include "AnimationManager.hpp"

#include "AnimationResource.hpp"
#include "AnimationComponent.hpp"

#include <Scene/Scene.hpp>
#include <Job/Job.hpp>
#include <Utils/Assert.hpp>

namespace Darius::Animation
{

	bool								_initialized = false;

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Register resources
		AnimationResource::Register();

		// Register component
		AnimationComponent::StaticConstructor();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	void Update(float dt)
	{
		auto& world = D_WORLD::GetRegistry();

		world.each([&](AnimationComponent& animationComponent)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						animationComponent.Update(dt);

					});
			}
		);

		if (D_JOB::IsMainThread())
			Darius::Job::WaitForThreadsToFinish();
	}

}
