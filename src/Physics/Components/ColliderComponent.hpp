#pragma once

#include "Physics/PhysicsManager.hpp"
#include "Physics/Resources/PhysicsMaterialResource.hpp"
#include "Physics/PhysicsActor.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "ColliderComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class DClass(Serialize) ColliderComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(ColliderComponent, ComponentBase, "Physics/Collider", false);

	public:

		// Events
		virtual void										Awake() override;
		virtual void										OnDestroy() override;

		virtual void										PreUpdate(bool simulating);

		virtual void										OnActivate() override;
		virtual void										OnDeactivate() override;
		virtual void										OnDeserialized() override;

#ifdef _D_EDITOR
		virtual bool										DrawDetails(float params[]) override;
#endif

		INLINE physx::PxShape*								GetShape() const { return mShape; }
		INLINE bool											IsDynamic() const { return mDynamic; }

		void												SetMaterial(PhysicsMaterialResource* material);
		INLINE PhysicsMaterialResource*						GetMaterial() const { return mMaterial.Get(); }

		virtual INLINE D_MATH::Quaternion					GetBiasedRotation() const { return D_MATH::Quaternion::Identity; }

	protected:
		virtual INLINE physx::PxGeometry const*				GetPhysicsGeometry() const { return nullptr; };
		virtual INLINE physx::PxGeometry*					UpdateAndGetPhysicsGeometry(bool& changed) { changed = false; return nullptr; };

	private:
		friend class PhysicsScene;

		void												InvalidatePhysicsActor();
		void												ReloadMaterialData();

		DField()
		bool												mDynamic;

		DField(Serialize)
		D_RESOURCE::ResourceRef<PhysicsMaterialResource>	mMaterial;

		PhysicsActor*										mActor;
		physx::PxShape*										mShape = nullptr;


	public:
		Darius_Physics_ColliderComponent_GENERATED

	};
}

File_ColliderComponent_GENERATED