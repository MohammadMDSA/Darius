#include "pch.hpp"
#include "AnimationManager.hpp"

#include "AnimationResource.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Animation
{

	bool								_initialized = false;

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		AnimationResource::Register();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

}
