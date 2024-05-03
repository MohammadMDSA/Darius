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
		mDynamicDirty(true),
		mDynamic(gameObject->HasComponent<RigidbodyComponent>())
	{ }

	PhysicsActor::~PhysicsActor()
	{
		UninitialzieActor();

		for(auto& [shape, _] : mColliders)
			if(shape->getReferenceCount() > 0)
				shape->release();
	}

	PhysicsActor* PhysicsActor::GetFromPxActor(physx::PxActor* actor)
	{
		if(!ActorMap.contains(actor))
			return nullptr;

		return ActorMap.at(actor);
	}

	void PhysicsActor::UninitialzieActor()
	{
		if(!mPxActor)
			return;

		ActorMap.erase(mPxActor);
		mScene->mPxScene->removeActor(*mPxActor);
		if(mPxActor->isReleasable())
			mPxActor->release();

		mPxActor = nullptr;
	}

	std::string GetGeometryTypeStr(physx::PxGeometryType::Enum type)
	{
		switch(type)
		{
		case physx::PxGeometryType::eSPHERE:
			return "Shape";
		case physx::PxGeometryType::ePLANE:
			return "Plane";
		case physx::PxGeometryType::eCAPSULE:
			return "Capsule";
		case physx::PxGeometryType::eBOX:
			return "Box";
		case physx::PxGeometryType::eCONVEXMESH:
			return "Convex Mesh";
		case physx::PxGeometryType::ePARTICLESYSTEM:
			return "Particle System";
		case physx::PxGeometryType::eTETRAHEDRONMESH:
			return "TetrahedronMesh";
		case physx::PxGeometryType::eTRIANGLEMESH:
			return "Triangle Mesh";
		case physx::PxGeometryType::eHEIGHTFIELD:
			return "Height Field";
		case physx::PxGeometryType::eHAIRSYSTEM:
			return "Hair";
		case physx::PxGeometryType::eCUSTOM:
			return "Custom";
		default:
			return "";
		}
	}

	physx::PxShape* PhysicsActor::AddCollider(ColliderComponent const* refComponent)
	{
		auto name = refComponent->GetComponentName();
		auto search = mCollidersLookup.find(name);
		if(search != mCollidersLookup.end())
			return search->second;

		auto physics = D_PHYSICS::GetCore();
		auto geom = refComponent->GetPhysicsGeometry();

		D_ASSERT(geom);

		physx::PxShape* shape = physics->createShape(*geom, *refComponent->GetMaterial(), true);

		if(!shape)
			return nullptr;

		auto geomType = geom->getType();
		if(IsGeometryCompatible(geomType))
			D_VERIFY(mPxActor->attachShape(*shape));
		else
			D_LOG_WARN(std::format("Dyanmic actor is not compatible with geometry type {}", GetGeometryTypeStr(geomType)));

		mColliders.insert({shape, refComponent->GetComponentName()});
		mCollidersLookup[name] = shape;
		shape->release();

		return shape;
	}

	physx::PxShape* PhysicsActor::GetShape(D_CORE::StringId const& compName)
	{
		auto search = mCollidersLookup.find(compName);
		if(search == mCollidersLookup.end())
			return nullptr;

		return search->second;
	}

	void PhysicsActor::RemoveCollider(ColliderComponent const* refComponent)
	{
		D_ASSERT(refComponent);

		auto compName = refComponent->GetComponentName();
		if(!mCollidersLookup.contains(compName))
			return;

		auto shape = GetShape(compName);
		D_ASSERT(shape);
		mCollidersLookup.erase(compName);
		mColliders.erase(shape);

		if(shape->getActor() == mPxActor)
			mPxActor->detachShape(*shape);

		D_ASSERT(mColliders.size() == mCollidersLookup.size());

		RemoveActorIfNecessary();
	}

	bool PhysicsActor::RemoveActorIfNecessary()
	{
		if(mColliders.size() == 0)
		{
			mScene->RemoveActor(this);
			mValid = false;
			return true;
		}

		return false;
	}

	void PhysicsActor::SetDynamic(bool dynamic)
	{
		if(mDynamic == dynamic)
			return;

		mDynamic = dynamic;
		mDynamicDirty = true;
	}

	void PhysicsActor::ForceRemoveActor()
	{
		mScene->RemoveActor(this);
		mValid = false;
	}

	bool PhysicsActor::IsGeometryCompatible(physx::PxGeometryType::Enum type)
	{
		switch(type)
		{
		case PxGeometryType::Enum::eSPHERE:
		case PxGeometryType::Enum::eCAPSULE:
		case PxGeometryType::Enum::eBOX:
		case PxGeometryType::Enum::eCONVEXMESH:
		case PxGeometryType::Enum::ePLANE:
			return true;
		default:
			return !mDynamic;
		}
	}

	void PhysicsActor::TransferShapes(physx::PxRigidActor* actor)
	{
		D_ASSERT(mPxActor);
		D_ASSERT(actor);

		D_ASSERT((UINT)mPxActor->getNbShapes() == (UINT)mColliders.size());

		for(auto& [shape, _] : mColliders)
		{
			bool compatible = IsGeometryCompatible(shape->getGeometry().getType());

			if(compatible)
				shape->acquireReference();

			mPxActor->detachShape(*shape);

			if(!compatible)
				continue;

			D_ASSERT(actor->attachShape(*shape));
			shape->release();
		}
	}

	void PhysicsActor::InitializeActor()
	{
		D_ASSERT(mGameObject && mGameObject->IsValid());

		if(mPxActor && !mDynamicDirty)
			return;

		auto physics = D_PHYSICS::GetCore();

		// Create actor initial transform
		auto mainTransform = mGameObject->GetTransform()->GetTransformData();
		auto transform = D_PHYSICS::GetTransform(mainTransform);

		physx::PxRigidActor* newActor;
		// Create proper actor if doesn't exist
		if(mDynamic)
		{
			newActor = physics->createRigidDynamic(transform);
		}
		else
		{
			newActor = physics->createRigidStatic(transform);
		}

		if(mPxActor)
			TransferShapes(newActor);

		UninitialzieActor();

		mPxActor = newActor;

		ActorMap.emplace(mPxActor, this);

		mScene->mPxScene->addActor(*mPxActor);

		mValid = true;
		mDynamicDirty = false;
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
