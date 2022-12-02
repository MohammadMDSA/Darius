#pragma once

#include "Physics/PhysicsManager.hpp"
#include "Physics/Resources/PhysicsMaterialResource.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class ColliderComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(ColliderComponent, ComponentBase, "Physics/Collider", false, false);

	public:

		// Events
		virtual void				Start() override;
		virtual void				OnDestroy() override;

		virtual void				PreUpdate(bool simulating);

		virtual void				OnActivate() override;
		virtual void				OnDeactivate() override;

#ifdef _D_EDITOR
		virtual bool				DrawDetails(float params[]) override;
#endif

		// Serialization
		virtual void				Serialize(D_SERIALIZATION::Json& json) const override;
		virtual void				Deserialize(D_SERIALIZATION::Json const& json) override;

		INLINE physx::PxShape*		GetShape() { return mShape; }

		D_H_COMP_REF_PROP(PhysicsMaterialResource, Material, SetPxShapeMaterial(););

		D_CH_FIELD(physx::PxShape*,			Shape = nullptr);
		D_CH_R_FIELD(bool,					Dynamic)

	protected:
		virtual INLINE physx::PxGeometry const* GetPhysicsGeometry() const { return nullptr; };
		virtual INLINE physx::PxGeometry* UpdateAndGetPhysicsGeometry(bool& changed) { changed = false; return nullptr; };

	private:
		friend class PhysicsScene;

		void						InvalidatePhysicsActor();
		INLINE void					SetPxShapeMaterial()
		{
			if (!mShape)
				return;
			physx::PxMaterial* mats[] = { const_cast<physx::PxMaterial*>(mMaterial.Get()->GetMaterial()) };
			mShape->setMaterials(mats, 1);
		}
	};
}
