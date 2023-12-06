#pragma once

#include "Physics/CollisionCommon.hpp"
#include "Physics/HitResult.hpp"
#include "Physics/PhysicsManager.hpp"
#include "Physics/Resources/PhysicsMaterialResource.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "ColliderComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	D_H_SIGNAL_COMP_FOUR_PARAM(CollisionSignalType, ColliderComponent*, thisCollider, ColliderComponent*, otherCollider, D_SCENE::GameObject*, otherGameObject, HitResult const&, Hit);
	D_H_SIGNAL_COMP_TWO_PARAM(TriggerSignalType, ColliderComponent*, thisCollider, D_SCENE::GameObject*, otherGameObject);

	class DClass(Serialize) ColliderComponent : public D_ECS_COMP::ComponentBase
	{
		GENERATED_BODY();
		D_H_COMP_BODY(ColliderComponent, ComponentBase, "Physics/Collider", false);

	public:

		// Physics Events
		CollisionSignalType									OnColliderContactEnter;
		CollisionSignalType									OnColliderContactLost;
		CollisionSignalType									OnColliderContactStay;

		TriggerSignalType									OnTriggerEnter;
		TriggerSignalType									OnTriggerExit;

		// State Events
		virtual void										Awake() override;
		virtual void										OnPreDestroy() override;

		virtual void										PreUpdate(bool simulating);

		virtual void										OnActivate() override;
		virtual void										OnDeactivate() override;
		virtual void										OnDeserialized() override;

		// Call when all the parameters are correctly set. Make sure to provide appropriate PxGeometry type for each component.
		INLINE virtual bool									CalculateGeometry(_OUT_ physx::PxGeometry & geom) const { return false; }
		INLINE virtual bool									UpdateGeometry() { return false; }

#ifdef _D_EDITOR
		virtual bool										DrawDetails(float params[]) override;
#endif

		INLINE physx::PxShape* GetShape() const { return mShape; }
		INLINE bool											IsDynamic() const { return mDynamic; }

		void												SetMaterial(PhysicsMaterialResource * material);
		void												SetTrigger(bool trigger);
		INLINE PhysicsMaterialResource* GetMaterial() const { return mMaterial.Get(); }
		INLINE bool											IsTrigger() const { return mTrigger; }

		virtual INLINE D_MATH::Quaternion					GetBiasedRotation() const { return D_MATH::Quaternion::Identity; }
		INLINE D_MATH::Vector3 const& GetUsedScale() const { return mUsedScale; }


	protected:
		virtual INLINE physx::PxGeometry const* GetPhysicsGeometry() const { return nullptr; };
		virtual INLINE void									CalculateScaledParameters() { mUsedScale = GetTransform()->GetScale(); }

		void												InvalidatePhysicsActor();
	private:
		friend class PhysicsScene;

		physx::PxGeometry const* UpdateAndGetPhysicsGeometry(bool& changed);
		void												ReloadMaterialData();

		DField()
			bool												mDynamic;

		DField(Serialize)
			bool												mTrigger;

		DField(Serialize)
			D_RESOURCE::ResourceRef<PhysicsMaterialResource>	mMaterial;

		PhysicsActor* mActor;
		physx::PxShape* mShape = nullptr;
		D_MATH::Vector3										mUsedScale;


	};
}

File_ColliderComponent_GENERATED