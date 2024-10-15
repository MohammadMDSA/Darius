#pragma once

#include "TextureResource.hpp"

#include <Core/Containers/Vector.hpp>
#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>
#include <Graphics/GraphicsUtils/SamplerManager.hpp>
#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceRef.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

#include "GenericMaterialResource.generated.hpp"

namespace Darius::Renderer
{
	struct MaterialTextureSlotInitializer;
	struct MaterialSamplerSlotInitializer;

	struct DStruct(Serialize) MaterialTextureSlot
	{
		GENERATED_BODY();

	public:
		MaterialTextureSlot() = default;
		MaterialTextureSlot(MaterialTextureSlotInitializer const& initializer);

		MaterialTextureSlot& operator= (MaterialTextureSlotInitializer const& initializer);

	public:
		DField(Serialize)
		D_CORE::StringId								Name;

		DField(Serialize)
		D_RESOURCE::ResourceRef<TextureResource>		Resource;

		std::shared_ptr<D_CORE::SignalScopedConnection> ChangeConnection;
	};

	struct MaterialTextureSlotInitializer
	{
	public:
		D_CORE::StringId								Name;

		INLINE bool operator== (MaterialTextureSlot const& slot) const
		{
			return Name == slot.Name;
		}
	};

	struct DStruct(Serialize) MaterialSamplerSlot
	{
		GENERATED_BODY();

	public:
		MaterialSamplerSlot() = default;
		MaterialSamplerSlot(MaterialSamplerSlotInitializer const& initializer);

		MaterialSamplerSlot& operator= (MaterialSamplerSlotInitializer const& initializer);

	public:
		DField(Serialize)
		D_CORE::StringId								Name;

		DField(Serialize)
		D_GRAPHICS_UTILS::SamplerDesc					SamplerDescription;
	};

	struct MaterialSamplerSlotInitializer
	{
	public:
		D_CORE::StringId								Name;

		INLINE bool operator== (MaterialSamplerSlot const& slot) const
		{
			return Name == slot.Name;
		}
	};

	class DClass(Serialize, Resource) GenericMaterialResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(GenericMaterialResource, "Generic Material", "");

	public:

		INLINE D3D12_GPU_DESCRIPTOR_HANDLE		GetTexturesHandle() const { return mTexturesTable; }
		INLINE D3D12_GPU_DESCRIPTOR_HANDLE		GetSamplersHandle() const { return mSamplerTable; }
		INLINE D3D12_GPU_VIRTUAL_ADDRESS		GetConstantsGpuAddress() const { return mConstantsGPU.GetGpuVirtualAddress(); }

		void									SetTextureCount(uint32_t textureCount);
		INLINE uint32_t							GetTextureCount() const { return mTextureCount; }
		virtual void							SetTexture(TextureResource* texture, uint32_t textureIndex);
		void									SetTextureSlot(MaterialTextureSlotInitializer const& data, uint32_t index);
		INLINE D_CORE::StringId					GetTextureName(uint32_t index) const { return index < mTextureCount ? D_CORE::StringId("") : mTextures[index].Name; }
		INLINE TextureResource*					GetTexture(uint32_t index) const { return index < mTextureCount ? mTextures[index].Resource.Get() : nullptr; }
		INLINE bool								IsTexturePresent(uint32_t index) const { return GetTexture(index) != nullptr; }
		virtual void							SetSamplerCount(uint32_t samplerCount);
		INLINE uint32_t							GetSamplerCount() const { return mSamplerCount; }
		void									SetSampler(D_GRAPHICS_UTILS::SamplerDesc const& sampler, uint32_t samplerIndex);
		void									SetSamplerSlot(MaterialSamplerSlotInitializer const&, uint32_t index);
		INLINE D_CORE::StringId					GetSamplerName(uint32_t index) const { return index < mTextureCount ? D_CORE::StringId("") : mTextures[index].Name; };
		INLINE D_GRAPHICS_UTILS::SamplerDesc	GetSampler(uint32_t index) const { return index < mSamplerCount ? mSamplers[index].SamplerDescription : D_GRAPHICS_UTILS::SamplerDesc(); }

		virtual bool							AreDependenciesDirty() const override;
		INLINE virtual bool						IsPrepared() const { return true; }
		virtual size_t							GetConstantsBufferSize() const { D_ASSERT_NOENTRY(); return 0; }
		virtual void const*						GetConstantsBufferData() const { D_ASSERT_NOENTRY(); return 0; }

#ifdef _D_EDITOR
		virtual bool							DrawDetails(float params[]) override { return false; }
#endif // _D_EDITOR


	protected:

		GenericMaterialResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource * parent, bool isDefault = false);

		// This method returns the fallback texture for in case there was a missing or invalid texture in the texture list of the material
		virtual TextureResource*				GetFallbackTexture(uint32_t textureIndex) const;
		virtual bool							UploadToGpu() override;
		INLINE virtual void						Unload() override { EvictFromGpu(); }

		virtual bool							WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void							SerializeTextures(D_SERIALIZATION::Json& j) const;
		virtual void							SerializeSamplers(D_SERIALIZATION::Json& j) const;
		virtual void							ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual void							DeserializeTextures(D_SERIALIZATION::Json const& j);
		virtual void							DeserializeSamplers(D_SERIALIZATION::Json const& j);

		INLINE virtual void						OnTextureDataChanged(uint32_t index) {}

		DField(Serialize)
		D_CONTAINERS::DVector<MaterialTextureSlot>	mTextures;

		DField(Serialize)
		D_CONTAINERS::DVector<MaterialSamplerSlot>	mSamplers;

		D_GRAPHICS_BUFFERS::UploadBuffer		mConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mConstantsGPU;
		D_GRAPHICS_MEMORY::DescriptorHandle		mTexturesTable;
		D_GRAPHICS_MEMORY::DescriptorHandle		mSamplerTable;

	private:

		uint32_t								mTextureCount : 16;
		uint32_t								mSamplerCount : 16;
		
	};
}