#include "pch.hpp"
#include "FBXSystem.hpp"

#include "FBXPrefabResource.hpp"
#include "FBXResource.hpp"

bool										_initialized = false;

namespace Darius::Fbx
{
	void Initialize(D_SERIALIZATION::Json const&)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		FBXPrefabResource::Register();
		FBXResource::Register();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}
}
