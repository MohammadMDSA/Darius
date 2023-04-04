#pragma once

#include <ResourceManager/Resource.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

#include <Physics/PhysicsManager.hpp>

namespace Darius::Physics
{
	class PhysicsMaterialResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(PhysicsMaterialResource, "Physics Material", ".physmat");

	public:

		INLINE physx::PxMaterial*		ModifyMaterial() { MakeDiskDirty(); return mPxData; }
		INLINE physx::PxMaterial const* GetMaterial() const { return mPxData; }

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		INLINE virtual bool				UploadToGpu() override { return true; }
		virtual void					Unload() override;

		INLINE operator physx::PxMaterial const& () const { return *mPxData; }

		D_CH_FIELD(physx::PxMaterial*, PxData)

	protected:
		PhysicsMaterialResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false);

	};
}
