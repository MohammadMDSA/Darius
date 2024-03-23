#include "pch.hpp"
#include "PhysicsActor.hpp"

#include "PhysicsManager.hpp"
#include "PhysicsScene.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/RigidbodyComponent.hpp"

#include <Core/Containers/Map.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "PhysicsActor.sgenerated.hpp"

using namespace physx;
using namespace D_CONTAINERS;


namespace Darius::Physics
{

	DUnorderedMap<PxActor*, PhysicsActor*> ActorMap;

	PhysicsActor::PhysicsActor(D_SCENE::GameObject const* gameObject, PhysicsScene* scene) :
		mPxActor(nullptr),
		mGameObject(gameObject),
		mScene(scene),
		mDynamic(gameObject->HasComponent<RigidbodyComponent>())
	{
	}

	PhysicsActor::~PhysicsActor()
	{
		UninitialzieActor();
	}

	PhysicsActor* PhysicsActor::GetFromPxActor(physx::PxActor* actor)
	{
		if (!ActorMap.contains(actor))
			return nullptr;

		return ActorMap.at(actor);
	}

	void PhysicsActor::UninitialzieActor()
	{
		if (!mPxActor)
			return;

		ActorMap.erase(mPxActor);
		mScene->mPxScene->removeActor(*mPxActor);
		if (mPxActor->isReleasable())
			mPxActor->release();

		mPxActor = nullptr;
	}

	physx::PxShape* PhysicsActor::AddCollider(ColliderComponent const* refComponent)
	{
		auto name = refComponent->GetComponentName();
		auto search = mCollidersLookup.find(name);
		if (search != mCollidersLookup.end())
			return search->second;

		auto physics = D_PHYSICS::GetCore();
		auto geom = refComponent->GetPhysicsGeometry();

		D_ASSERT(geom);

		physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*mPxActor, *geom, *refComponent->GetMaterial());

		D_ASSERT(shape);

		mColliders[shape] = refComponent->GetComponentName();
		mCollidersLookup[name] = shape;

		return shape;
	}

	physx::PxShape* PhysicsActor::GetShape(std::string const& compName)
	{
		auto search = mCollidersLookup.find(compName);
		if (search == mCollidersLookup.end())
			return nullptr;

		return search->second;
	}

	void PhysicsActor::RemoveCollider(ColliderComponent const* refComponent)
	{
		D_ASSERT(refComponent);

		auto compName = refComponent->GetComponentName();
		if (!mCollidersLookup.contains(compName))
			return;

		auto shape = GetShape(compName);
		D_ASSERT(shape);
		mCollidersLookup.erase(compName);
		mColliders.erase(shape);
		mPxActor->detachShape(*shape);

		D_ASSERT(mColliders.size() == mCollidersLookup.size());

		RemoveActorIfNecessary();
	}

	bool PhysicsActor::RemoveActorIfNecessary()
	{
		if (!IsDynamic() && mColliders.size() == 0)
		{
			mScene->RemoveActor(this);
			mValid = false;
			return true;
		}

		return false;
	}

	void PhysicsActor::ForceRemoveActor()
	{
		mScene->RemoveActor(this);
		mValid = false;
	}

	void PhysicsActor::TransferShapes(physx::PxRigidActor* actor)
	{
		D_ASSERT(mPxActor);
		D_ASSERT(actor);

		D_ASSERT((UINT)mPxActor->getNbShapes() == (UINT)mColliders.size());
	}

	void PhysicsActor::InitializeActor()
	{
		D_ASSERT(mGameObject && mGameObject->IsValid());

		if (mPxActor)
			return;

		auto physics = D_PHYSICS::GetCore();

		// Create actor initial transform
		auto mainTransform = mGameObject->GetTransform()->GetTransformData();
		auto transform = D_PHYSICS::GetTransform(mainTransform);

		physx::PxRigidActor* newActor;
		// Create proper actor if doesn't exist
		if (mDynamic)
		{
			newActor = physics->createRigidDynamic(transform);
		}
		else
		{
			newActor = physics->createRigidStatic(transform);
		}

		if (mPxActor)
			TransferShapes(newActor);

		UninitialzieActor();

		mPxActor = newActor;

		ActorMap.emplace(mPxActor, this);

		mScene->mPxScene->addActor(*mPxActor);

		mValid = true;
	}

	void PhysicsActor::PreUpdate()
	{
		auto trans = mGameObject->GetTransform();
		auto rot = trans->GetRotation();
		auto pos = trans->GetPosition();
		mPxActor->setGlobalPose(physx::PxTransform(GetVec3(pos), GetQuat(rot)));
	}

	void PhysicsActor::Update()
	{
		auto preTrans = mGameObject->GetTransform();
		D_MATH::Transform physicsTrans = D_PHYSICS::GetTransform(mPxActor->getGlobalPose());

		physicsTrans.Scale = preTrans->GetScale();
		
		preTrans->SetWorld(D_MATH::Matrix4(physicsTrans.GetWorld()));
	}
}
