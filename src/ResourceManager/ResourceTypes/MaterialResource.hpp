#pragma once

#include "Resource.hpp"

#include <Renderer/FrameResource.hpp>
#include <Renderer/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp>


#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_RENDERER_FRAME_RESOUCE;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_CORE;

namespace Darius::ResourceManager
{
	class DResourceManager;

	class MaterialResource : public Resource
	{
		D_CH_RESOUCE_BODY(MaterialResource, ResourceType::Material)
		
	public:
		INLINE Material* ModifyData() { MakeDiskDirty(); MakeGpuDirty(); return &mMaterial; }
		INLINE const Material*			GetData() const { return &mMaterial; }

		INLINE virtual ResourceType		GetType() const override { return ResourceType::Material; }
		
		virtual bool					SuppoertsExtension(std::wstring ext) override;

		INLINE operator const Material* () const { return &mMaterial; }
		INLINE operator Material*() { ModifyData(); }

		INLINE operator D3D12_GPU_VIRTUAL_ADDRESS const() { return mMaterialConstantsGPU.GetGpuVirtualAddress(); }
			
		D_CH_FIELD(Material, Material);

	protected:

		virtual void						WriteResourceToFile() const override;
		virtual void						ReadResourceFromFile() override;
		virtual void						UploadToGpu(D_GRAPHICS::GraphicsContext& context) override;

	private:
		friend class DResourceManager;

		MaterialResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, id, isDefault) {}

		D_GRAPHICS_BUFFERS::UploadBuffer	mMaterialConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMaterialConstantsGPU;

	};
}