#pragma once

#include "GenericMaterialResource.hpp"
#include "ShaderResource.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

#include "ShaderMaterialResource.generated.hpp"

namespace Darius::Renderer
{
	
	class DClass(Serialize, Resource) ShaderMaterialResource : public GenericMaterialResource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(ShaderMaterialResource, "Shader Material", ".smat");

	public:

		virtual ~ShaderMaterialResource();

#if _D_EDITOR
		virtual bool							DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual size_t							GetConstantsBufferSize() const { return mConstantBufferMemory ? mConstantBufferMemory->mSize : 0; }
		virtual void const*						GetConstantsBufferData() const { return mConstantBufferMemory->GetDataPtr(); }

		virtual void							SetShader(ShaderResource* shader);

	protected:

		ShaderMaterialResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false);


		virtual bool							WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void							ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;

		virtual void							OnShaderChanged(ShaderResource* shader, std::shared_ptr< ShaderConstantParameters> preConstantParams);

		virtual bool							IsPrepared() const override;
		virtual void							SerializeConstants(D_SERIALIZATION::Json& constants) const;
		virtual void							DeserializeConstants(D_SERIALIZATION::Json const& constants);

	private:

		class ConstantBufferData
		{
		public:
			ConstantBufferData(size_t size);
			~ConstantBufferData();

			INLINE size_t						GetSize() const { return mSize; }
			INLINE void*						GetDataPtr() const { return mData; }

			INLINE ShaderConstantPropertyBuffer	GetShaderConstantPropertyBuffer() const { return { mData, mSize }; }

		private:
			size_t								mSize;
			void*								mData;

			friend ShaderMaterialResource;
		};

		using GenericMaterialResource::SetTextureCount;
		using GenericMaterialResource::SetTexture;
		using GenericMaterialResource::SetTextureSlot;
		using GenericMaterialResource::SetSamplerCount;
		using GenericMaterialResource::SetSampler;
		using GenericMaterialResource::SetSamplerSlot;

		DField(Serialize)
		D_RESOURCE::ResourceRef<ShaderResource>	mShader;

		std::unique_ptr<ConstantBufferData>		mConstantBufferMemory;
		D_CORE::SignalConnection				mShaderCompileConnection;

		D_SERIALIZATION::Json					mSerializedConstants;

	};
}