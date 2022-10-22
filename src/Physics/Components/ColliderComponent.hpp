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
		D_H_COMP_BODY(ColliderComponent, ComponentBase, "Collider", false, false);

	public:

		virtual void				Start() override;
		virtual void				OnDestroy() override;

		virtual void				Update(float dt) override;
		virtual void				PreUpdate();

		virtual bool				DrawDetails(float params[]) override;

		virtual void                Serialize(D_SERIALIZATION::Json&) const override;
		virtual void                Deserialize(D_SERIALIZATION::Json const&) override;

		INLINE bool					IsDynamic() const { return !mIsStatic; }

		D_CH_FIELD(bool, IsStatic = false);

		D_CH_FIELD(physx::PxRigidActor*,	Actor = nullptr);
		D_CH_FIELD(physx::PxShape*,			Shape = nullptr);

	protected:
		virtual INLINE physx::PxGeometry const* GetPhysicsGeometry() const { return nullptr; };
		virtual INLINE physx::PxGeometry* UpdateAndGetPhysicsGeometry(bool& changed) { changed = false; return nullptr; };

	private:

		void						InitActor();

		void						EnablePhysics();
		void						DisablePhysics();


	};
}
