#pragma once

#include <ResourceManager/ResourceTypes/Resource.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

#include <Physics/PhysicsManager.hpp>

using namespace D_RESOURCE;

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

		virtual void					WriteResourceToFile() const override;
		virtual void					ReadResourceFromFile() override;
		virtual bool					UploadToGpu(D_GRAPHICS::GraphicsContext& context);
		virtual void					Unload() override;

		INLINE operator physx::PxMaterial const& () const { return *mPxData; }

		D_CH_FIELD(physx::PxMaterial*, PxData)

	protected:
		PhysicsMaterialResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false);

	};
}
