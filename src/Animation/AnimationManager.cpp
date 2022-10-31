#include "pch.hpp"
#include "AnimationManager.hpp"

#include "AnimationResource.hpp"
#include "AnimationComponent.hpp"

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

}
