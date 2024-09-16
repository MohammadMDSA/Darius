#pragma once

#include <ResourceManager/ResourceRef.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

#include <Physics/PhysicsManager.hpp>

#include "PhysicsMaterialResource.generated.hpp"

namespace Darius::Physics
{
	class DClass(Serialize, Resource) PhysicsMaterialResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(PhysicsMaterialResource, "Physics Material", ".physmat");

	public:

		INLINE physx::PxMaterial*		ModifyMaterial() { MakeDiskDirty(); return mPxData; }
		INLINE physx::PxMaterial const* GetMaterial() const { return mPxData; }

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// Static Friction
		void							SetStaticFriction(float value);
		INLINE float					GetStaticFriction() const { return mPxData->getStaticFriction(); }

		// Dynamic Friction
		void							SetDynamicFriction(float value);
		INLINE float					GetDynamicFriction() const { return mPxData->getDynamicFriction(); }

		// Restitution
		void							SetRestitution(float value);
		INLINE float					GetRestitution() const { return mPxData->getRestitution(); }

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		INLINE virtual bool				UploadToGpu() override { return true; }
		virtual void					Unload() override;

		INLINE operator physx::PxMaterial const& () const { return *mPxData; }

		INLINE virtual bool				AreDependenciesDirty() const override { return false; }

	protected:
		PhysicsMaterialResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false);

	private:
		physx::PxMaterial*				mPxData;

	};
}

File_PhysicsMaterialResource_GENERATED
