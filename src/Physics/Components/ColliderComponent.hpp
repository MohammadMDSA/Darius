#pragma once

#include "Physics/PhysicsManager.hpp"

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

		INLINE physx::PxShape*		GetShape() { return mShape; }
		INLINE physx::PxMaterial const* GetMaterial() const { return D_PHYSICS::GetDefaultMaterial(); }

		D_CH_FIELD(physx::PxShape*,			Shape = nullptr);
		D_CH_R_FIELD(bool,					Dynamic)

	protected:
		virtual INLINE physx::PxGeometry const* GetPhysicsGeometry() const { return nullptr; };
		virtual INLINE physx::PxGeometry* UpdateAndGetPhysicsGeometry(bool& changed) { changed = false; return nullptr; };

	private:
		friend class PhysicsScene;

		void						InvalidatePhysicsActor();
	};
}
