#pragma once

#include "Physics/PhysicsScene.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class RigidbodyComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(RigidbodyComponent, ComponentBase, "Physics/Rigidbody", true, false);

	public:
		virtual INLINE bool				IsDisableable() const { return false; }

		// Events
		virtual void					Start() override;
		virtual void					OnDestroy() override;
		virtual void					Update(float) override;
		virtual void					PreUpdate();

	protected:
		D_CH_FIELD(physx::PxRigidDynamic*,	Actor = nullptr);
	private:

	};
}
