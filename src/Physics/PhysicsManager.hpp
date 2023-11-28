#pragma once

#include "PhysicsScene.hpp"

#include <Math/VectorMath.hpp>
#include <ResourceManager/Resource.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	void					Initialize(D_SERIALIZATION::Json const& settings);
	void					Shutdown();

#ifdef _D_EDITOR
	bool					OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

	void					Update(bool running);

	PhysicsScene*			GetScene();
	physx::PxPhysics*		GetCore();
	D_RESOURCE::ResourceHandle GetDefaultMaterial();

	INLINE physx::PxQuat	GetQuat(D_MATH::Quaternion const& quat)
	{
		return physx::PxQuat(quat.GetX(), quat.GetY(), quat.GetZ(), quat.GetW());
	}

	INLINE D_MATH::Quaternion GetQuat(physx::PxQuat const& quat)
	{
		return D_MATH::Quaternion(quat.x, quat.y, quat.z, quat.w);
	}
	
	INLINE physx::PxVec3	GetVec3(D_MATH::Vector3 const& vec3)
	{
		return physx::PxVec3(vec3.GetX(), vec3.GetY(), vec3.GetZ());
	}

	INLINE D_MATH::Vector3	GetVec3(physx::PxVec3 const& vec3)
	{
		return D_MATH::Vector3(vec3.x, vec3.y, vec3.z);
	}

	INLINE physx::PxVec4	GetVec4(D_MATH::Vector4 const& vec4)
	{
		return physx::PxVec4(vec4.GetX(), vec4.GetY(), vec4.GetZ(), vec4.GetW());
	}

	INLINE D_MATH::Vector4	GetVec4(physx::PxVec4 const& vec4)
	{
		return D_MATH::Vector4(vec4.x, vec4.y, vec4.z, vec4.w);
	}

	INLINE physx::PxTransform GetTransform(D_MATH::Transform const& trans)
	{
		return physx::PxTransform(GetVec3(trans.Translation), GetQuat(trans.Rotation));
	}

	INLINE D_MATH::Transform GetTransform(physx::PxTransform const& trans)
	{
		return D_MATH::Transform(GetVec3(trans.p), GetQuat(trans.q));
	}

}
