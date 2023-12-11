#include "pch.hpp"
#include "AnimationManager.hpp"

#include "AnimationResource.hpp"
#include "AnimationComponent.hpp"

#include <Job/Job.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

namespace Darius::Animation
{

	bool								_initialized = false;

	void Initialize(D_SERIALIZATION::Json const& settings)
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
		D_CONTAINERS::DVector<std::function<void()>> updateFuncs;
		updateFuncs.reserve(D_WORLD::CountComponents<AnimationComponent>());

		D_WORLD::IterateComponents<AnimationComponent>([&](AnimationComponent& animationComponent)
			{
				updateFuncs.push_back([&]()
					{
						if (animationComponent.IsActive())
							animationComponent.Update(dt);

					});
			}
		);
		
		D_JOB::AddTaskSetAndWait(updateFuncs);
	}

}
