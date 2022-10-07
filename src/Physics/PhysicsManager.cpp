#include "pch.hpp"
#include "PhysicsManager.hpp"

#include <Utils/Assert.hpp>

#include <PxPhysics.h>
#include <PxPhysicsAPI.h>

using namespace physx;

namespace Darius::Physics
{
	bool										_init = false;

	physx::PxDefaultAllocator		gAllocator;
	physx::PxDefaultErrorCallback	gErrorCallback;

	physx::PxFoundation* gFoundation = NULL;
	physx::PxPhysics* gPhysics = NULL;

	physx::PxDefaultCpuDispatcher* gDispatcher = NULL;
	physx::PxScene* gScene = NULL;

	physx::PxMaterial* gMaterial = NULL;

	physx::PxPvd* gPvd = NULL;

	void Initialize()
	{
		D_ASSERT(!_init);
		_init = true;
	}

	void Shutdown()
	{
		D_ASSERT(_init);
	}
}
