#pragma once

#include <Core/Containers/Vector.hpp>
#include <Math/VectorMath.hpp>

#include "HitResult.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	struct DStruct(Reg) HitContactPoint
	{
		GENERATED_BODY();

		// The position of the contact point between the shapes, in world space.
		D_MATH::Vector3 Position;

		// The separation of the shapes at the contact point.  A negative separation denotes a penetration.
		float Separation;

		// The normal of the contacting surfaces at the contact point. The normal direction points from the second shape to the first shape.
		D_MATH::Vector3 Normal;

		// The impulse applied at the contact point, in world space. Divide by the simulation time step to get a force value.
		D_MATH::Vector3 Impulse;

	};

	struct DStruct(Reg) HitResult
	{

		GENERATED_BODY();

		DField()
		D_CONTAINERS::DVector<HitContactPoint> Contacts;
	};
}