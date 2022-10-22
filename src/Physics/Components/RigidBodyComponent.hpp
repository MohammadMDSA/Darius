#pragma once

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class RigidBodyComponent : D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(RigidBodyComponent, ComponentBase, "Rigid Body", true, false);

	public:

	private:
		
	};
}
