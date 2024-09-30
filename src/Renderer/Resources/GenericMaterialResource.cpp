#include "Graphics/pch.hpp"
#include "GenericMaterialResource.hpp"

#include "Renderer/RendererManager.hpp"

#include <Core/Memory/Memory.hpp>

#include "GenericMaterialResource.sgenerated.hpp"

#define MAX_TEXTURE_COUNT 16u
#define MAX_SAMPLER_COUNT 16u

using namespace D_GRAPHICS_UTILS;
using namespace D_SERIALIZATION;

namespace Darius::Renderer
{
	D_CH_RESOURCE_DEF(GenericMaterialResource);

	MaterialTextureSlot::MaterialTextureSlot(MaterialTextureSlotInitializer const& initializer) :
		Name(initializer.Name),
		Resource(nullptr) {}

	MaterialTextureSlot& MaterialTextureSlot::operator= (MaterialTextureSlotInitializer const& initializer)
	{
		this->Name = initializer.Name;
		return *this;
	}

	MaterialSamplerSlot::MaterialSamplerSlot(MaterialSamplerSlotInitializer const& initializer) :
		Name(initializer.Name),
		SamplerDescription({}) {}

	MaterialSamplerSlot& MaterialSamplerSlot::operator= (MaterialSamplerSlotInitializer const& initializer)
	{
		this->Name = initializer.Name;
		return *this;
	}

	GenericMaterialResource::GenericMaterialResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) :
		Resource(uuid, path, name, id, parent, isDefault),
		mTextureCount(0u),
		mSamplerCount(0u)
	{

	}

	void GenericMaterialResource::SetTextureCount(uint32_t textureCount)
	{
		if (textureCount == mTextureCount)
			return;

		if (textureCount > MAX_TEXTURE_COUNT)
		{
			D_LOG_WARN("Max texture count available for material is " << MAX_TEXTURE_COUNT << " but requested " << textureCount);
			D_LOG_WARN("Texture count set to max: " << MAX_TEXTURE_COUNT);
			mTextureCount = MAX_TEXTURE_COUNT;
		}
		else
			mTextureCount = textureCount;

		mTextures.resize(mTextureCount);

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	void GenericMaterialResource::SetTextureSlot(MaterialTextureSlotInitializer const& data, uint32_t textureIndex)
	{
		if (textureIndex >= mTextureCount)
		{
			D_LOG_WARN_FMT("Tried to set texture with index {}, but texture count is {}. Ignoring...", (int)textureIndex, (int)mTextureCount);

			return;
		}

		MaterialTextureSlot& dest = mTextures[textureIndex];
		if (data == dest)
			return;

		dest = data;

		MakeDiskDirty();

		SignalChange();
	}

	void GenericMaterialResource::SetSamplerSlot(MaterialSamplerSlotInitializer const& data, uint32_t samplerIndex)
	{
		if (samplerIndex >= mSamplerCount)
		{
			D_LOG_WARN_FMT("Tried to set sampler with index {}, but sampler count is {}. Ignoring...", (int)samplerIndex, (int)mSamplerCount);

			return;
		}

		MaterialSamplerSlot& dest = mSamplers[samplerIndex];
		if (data == dest)
			return;

		dest = data;

		MakeDiskDirty();

		SignalChange();
	}

	void GenericMaterialResource::SetSamplerCount(uint32_t samplerCount)
	{
		if (samplerCount == mSamplerCount)
			return;

		if (samplerCount > MAX_SAMPLER_COUNT)
		{
			D_LOG_WARN("Max sampler count available for material is " << MAX_SAMPLER_COUNT << " but requested " << samplerCount);
			D_LOG_WARN("Texture count set to max: " << MAX_SAMPLER_COUNT);
			mSamplerCount = MAX_SAMPLER_COUNT;
		}
		else
			mSamplerCount = samplerCount;

		mSamplers.resize(mSamplerCount);

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	TextureResource* GenericMaterialResource::GetFallbackTexture(uint32_t textureIndex) const
	{
		return D_RESOURCE::GetResourceSync<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::Texture2DBlackOpaque)).Get();
	}

	void GenericMaterialResource::SetTexture(TextureResource* texture, uint32_t textureIndex)
	{
		if (textureIndex >= mTextureCount)
		{
			D_LOG_WARN_FMT("Tried to set texture with index {}, but texture count is {}. Ignoring...", (int)textureIndex, (int)mTextureCount);

			return;
		}

		if (mTextures[textureIndex].Resource == texture)
			return;

		mTextures[textureIndex].Resource = texture;

		if (mTextures[textureIndex].Resource.IsValid())
		{
			mTextures[textureIndex].ChangeConnection = std::make_shared<D_CORE::SignalScopedConnection>(texture->SubscribeOnChange([&, textureIndex](D_RESOURCE::Resource* res)
				{
					OnTextureChanged(textureIndex);
				}));

			// Load if still not loaded
			if (!mTextures[textureIndex].Resource->IsLoaded())
			{

				D_RESOURCE_LOADER::LoadResourceAsync(texture, [&](auto _)
					{
						MakeGpuDirty();
					}, true);
			}
		}


		MakeGpuDirty();

		if (!IsLocked())
			MakeDiskDirty();

		SignalChange();
	}

	void GenericMaterialResource::SetSampler(SamplerDesc const& sampler, uint32_t samplerIndex)
	{
		if (samplerIndex >= mSamplerCount)
		{
			D_LOG_WARN_FMT("Tried to set sampler with index {}, but sampler count is {}. Ignoring...", (int)samplerIndex, (int)mSamplerCount);

			return;
		}

		if (mSamplers[samplerIndex].SamplerDescription == sampler)
			return;

		mSamplers[samplerIndex].SamplerDescription = sampler;

		MakeGpuDirty();

		if (!IsLocked())
			MakeDiskDirty();

		SignalChange();
	}

	bool GenericMaterialResource::AreDependenciesDirty() const
	{
		for (auto const& tex : mTextures)
		{
			if (tex.Resource.IsValidAndGpuDirty())
				return true;
		}
		return false;
	}

	bool GenericMaterialResource::WriteResourceToFile(Json& j) const
	{
		D_ASSERT_M(false, "Not Implemented");

		return false;
	}

	void GenericMaterialResource::SerializeTextures(Json& j) const
	{
		D_SERIALIZATION::Serialize(mTextures, j);
	}

	void GenericMaterialResource::SerializeSamplers(Json& j) const
	{
		D_SERIALIZATION::Serialize(mSamplers, j);
	}

	void GenericMaterialResource::ReadResourceFromFile(Json const& j, bool& dirtyDisk)
	{

	}

	bool GenericMaterialResource::UploadToGpu()
	{
		size_t constantsBufferSize = GetConstantsBufferSize();
		bool shouldCreateConstantBuffers = mConstantsCPU.GetBufferSize() != constantsBufferSize;
		if (shouldCreateConstantBuffers || mConstantsGPU.GetGpuVirtualAddress() == D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			// Initializing Material Constant Buffers
			mConstantsCPU.Create(std::wstring(L"Material Constants Upload Buffer") + GetName(), (uint32_t)constantsBufferSize, D_GRAPHICS_DEVICE::gNumFrameResources);
			mConstantsGPU.Create(std::wstring(L"Material Constants GPU Buffer: ") + GetName(), 1u, (uint32_t)constantsBufferSize, GetConstantsBufferData());
			mTexturesTable = D_RENDERER::AllocateTextureDescriptor(MAX_TEXTURE_COUNT);
			mSamplerTable = D_RENDERER::AllocateSamplerDescriptor(MAX_TEXTURE_COUNT);
		}

		uint32_t destCount = mTextureCount;
		D_CONTAINERS::DVector<UINT> sourceCount;
		D_CONTAINERS::DVector<D3D12_CPU_DESCRIPTOR_HANDLE> initialTextures;
		for (uint32_t i = 0; i < mTextureCount; i++)
		{
			sourceCount.push_back(1u);
			auto currentTex = mTextures[i].Resource;

			if (!currentTex.IsValid() || !currentTex->IsLoaded())
			{
				auto fallbackTexture = GetFallbackTexture(i);
				D_ASSERT(fallbackTexture && !fallbackTexture->IsDirtyGPU());
				initialTextures.push_back(fallbackTexture->GetTextureData()->GetSRV());
			}
			else
			{
				initialTextures.push_back(currentTex->GetTextureData()->GetSRV());
			}
		}
		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &mTexturesTable, &destCount, destCount, initialTextures.data(), sourceCount.data(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Loading samplers
		auto incSize = D_GRAPHICS_DEVICE::GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (uint32_t i = 0; i < mSamplerCount; i++)
		{
			mSamplers[i].SamplerDescription.CreateDescriptor(mSamplerTable + incSize * i);
		}

		// Uploading constants
		auto uploadData = mConstantsCPU.MapInstance(D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex());
		memcpy(uploadData, GetConstantsBufferData(), constantsBufferSize);
		mConstantsCPU.Unmap();

		auto& context = D_GRAPHICS::CommandContext::Begin(L"Material Resource Uploader");
		context.UploadToBuffer(mConstantsGPU, mConstantsCPU);
		context.TransitionResource(mConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);
		context.Finish();
		return true;
	}
}