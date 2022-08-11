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

namespace Darius::ResourceManager
{
	class DResourceManager;

	class MaterialResource : public Resource
	{
		D_CH_RESOUCE_BODY(MaterialResource, ResourceType::Material)
		
	public:
		INLINE Material*				GetData() { mDirtyDisk = mDirtyGPU = true; return &mMaterial; }
		INLINE const Material*			GetData() const { return &mMaterial; }

		INLINE virtual ResourceType		GetType() const override { return ResourceType::Material; }

		virtual bool					Save() override;
		virtual bool					Load() override;
		virtual void					UpdateGPU(D_GRAPHICS::GraphicsContext& context) override;
		virtual bool					SuppoertsExtension(std::wstring ext) override;

		INLINE operator const Material* () const { return &mMaterial; }
		INLINE operator Material*() { GetData(); }

		INLINE operator D3D12_GPU_VIRTUAL_ADDRESS const() { return mMaterialConstantsGPU.GetGpuVirtualAddress(); }
			
		D_CH_FIELD(Material, Material);
	private:
		friend class DResourceManager;

		MaterialResource(std::wstring path, DResourceId id, bool isDefault = false) :
			Resource(path, id, isDefault) {}

		D_GRAPHICS_BUFFERS::UploadBuffer	mMaterialConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMaterialConstantsGPU;

	};
}