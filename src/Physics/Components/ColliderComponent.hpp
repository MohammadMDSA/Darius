#pragma once

#include "Physics/CollisionCommon.hpp"
#include "Physics/HitResult.hpp"
#include "Physics/PhysicsManager.hpp"
#include "Physics/Resources/PhysicsMaterialResource.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

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
		virtual void										Start() override;
		virtual void										OnDestroy() override;

		virtual void										PreUpdate();

		virtual void										OnActivate() override;
		virtual void										OnDeactivate() override;
		virtual void										OnDeserialized() override;

		// Call when all the parameters are correctly set. Make sure to provide appropriate PxGeometry type for each component.
		INLINE virtual bool									CalculateGeometry(_OUT_ physx::PxGeometry & geom) const { return false; }
		INLINE virtual bool									UpdateGeometry() { return false; }
		INLINE PhysicsActor const*							GetPhysicsActor() const { return mActor.Get(); }

		void												UpdateShape();

#ifdef _D_EDITOR
		virtual bool										DrawDetails(float params[]) override;
#endif

		void												SetMaterial(PhysicsMaterialResource* material);
		void												SetTrigger(bool trigger);
		INLINE PhysicsMaterialResource*						GetMaterial() const { return mMaterial.Get(); }
		INLINE bool											IsTrigger() const { return mTrigger; }

		INLINE D_MATH::Vector3 const&						GetUsedScale() const { return mUsedScale; }

		INLINE D_MATH::Vector3 const&						GetCenterOffset() const { return mCenterOffset; }
		INLINE D_MATH::Vector3 const&						GetScaledCenterOffset() const { return mScaledCenterOffset; }

		void												SetCenterOffset(D_MATH::Vector3 const& centerOffset);

		virtual INLINE physx::PxGeometry const*				GetPhysicsGeometry() const { return nullptr; };

	protected:
		virtual void										CalculateScaledParameters();

		bool												InvalidatePhysicsActor();
	private:
		friend class PhysicsScene;

		physx::PxGeometry const*							UpdateAndGetPhysicsGeometry(bool& changed);
		void												ReloadMaterialData();

		INLINE void											OnTransformWorldChanged(D_MATH::TransformComponent* trans, D_MATH::Transform const& worldTransform)
		{
			if (!trans->GetScale().NearEquals(mUsedScale, COLLIDER_SCALE_TOLERANCE))
				SetDirty();
		}

#if _D_EDITOR
		void												OnTransformWorldChangedEditor(D_MATH::TransformComponent* trans, D_MATH::Transform const& worldTransform);
#endif

		DField(Serialize)
		bool												mTrigger;

		DField(Serialize)
		D_MATH::Vector3										mCenterOffset;


		DField(Serialize)
		D_RESOURCE::ResourceRef<PhysicsMaterialResource>	mMaterial;

		D_CORE::Ref<PhysicsActor>							mActor;
		D_MATH::Vector3										mUsedScale;
		D_MATH::Vector3										mScaledCenterOffset;

		D_CORE::SignalConnection							mTransformChangeSignalConnection;

#if _D_EDITOR
		D_CORE::SignalConnection							mEditorTransformChangeSignalConnection;
#endif
	};
}

File_ColliderComponent_GENERATED