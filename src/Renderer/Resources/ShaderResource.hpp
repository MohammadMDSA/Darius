#pragma once

#include <Core/Containers/Vector.hpp>
#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include "ShaderResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Graphics::Utils::Shaders
{
	class PixelShader;
}

namespace Darius::Renderer
{
	class ShaderResource;

	struct ShaderConstantPropertyBuffer
	{
		void*					BufferMemory;
		size_t					BufferSize;
	};

	class ShaderConstantProperty : public std::enable_shared_from_this<ShaderConstantProperty>
	{
	public:

		INLINE ShaderConstantProperty(D_CORE::StringId const& parameterName, size_t offset) :
			std::enable_shared_from_this<ShaderConstantProperty>(),
			mName(parameterName),
			mBufferOffset(offset)
		{}

		virtual size_t				GetSizeInConstantBuffer() const = 0;

		// Serialization
		virtual void				Serialize(D_SERIALIZATION::Json& json, void const* buffer) const = 0;
		virtual bool				Deserialize(D_SERIALIZATION::Json const& json, void* buffer) const = 0;

		virtual void*				GetValuePtr(void* buffer) const = 0;
		INLINE D_CORE::StringId		GetParameterName() const { return mName; }
		INLINE size_t				GetBufferOffset() const { return mBufferOffset; }

		INLINE void					CopyTo(ShaderConstantProperty* other, void* srcBuffer, void* destBuffer) const
		{
			memcpy(other->GetValuePtr(destBuffer), GetValuePtr(srcBuffer), GetSizeInConstantBuffer());
		}


#if _D_EDITOR
		virtual bool				DrawDetails(void* buffer) = 0;
		virtual std::string			GetParameterTypeName() const = 0;
#endif // _D_EDITOR

	private:

		const D_CORE::StringId		mName;
		const size_t				mBufferOffset;
	};

	class ShaderConstantParameters
	{
	public:

		void Serialize(D_SERIALIZATION::Json& constantsJson, ShaderConstantPropertyBuffer const& buffer) const;
		void Deserialize(D_SERIALIZATION::Json const& constantsJson, ShaderConstantPropertyBuffer const& buffer) const;

#if _D_EDITOR
		bool DrawDetails(ShaderConstantPropertyBuffer const& buffer);
#endif // _D_EDITOR

		bool AddProperty(std::shared_ptr<ShaderConstantProperty> prop);

		void CopyTo(ShaderConstantParameters* dest, ShaderConstantPropertyBuffer const& srcBuffer, ShaderConstantPropertyBuffer const& destBuffer) const;

	private:
		D_CONTAINERS::DUnorderedMap<D_CORE::StringId, std::shared_ptr<ShaderConstantProperty>> mPropertyMap;

		INLINE static std::string ClassName() { return "ShaderConstantParameters"; }
	};

	using ShaderCompileSignalCallback = void(ShaderResource*, std::shared_ptr<ShaderConstantParameters> preConstantParams);

	class DClass(Serialize, Resource) ShaderResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();
		D_CH_RESOURCE_BODY(ShaderResource, "Shader", ".hlsl");

	public:


#ifdef _D_EDITOR
		bool										DrawDetails(float params[]);
		bool										ReloadAndRecompile();
#endif

		INLINE virtual bool							AreDependenciesDirty() const override { return false; }

		INLINE ID3D12ShaderReflection*				GetReflectionData() const { return mShaderReflectionData.Get(); }

		NODISCARD D_CORE::SignalConnection			SubscribeOnCompiled(std::function<ShaderCompileSignalCallback> callback);

		bool										CompileShader(bool force);

		INLINE std::shared_ptr<ShaderConstantParameters> GetConstantParameters() const { return mConstantParams; }

		INLINE size_t								GetConstantParamsBufferSize() const { return mConstantParamsBufferSize; }

		static INLINE std::string					GetConstantBufferName() { return "$Globals"; }

	protected:
		ShaderResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false) :
			Resource(uuid, path, name, id, parent, isDefault)
		{}

		virtual bool								WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void								ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		INLINE virtual bool							UploadToGpu() override { return true; }

		virtual void								Unload() override;
		void										ConstructConstantParams();

	private:
		std::shared_ptr<D_CONTAINERS::DVector<std::byte>>	mCode;
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection>		mShaderReflectionData;
		std::string									mCompileMessage;

		D_CORE::Signal<ShaderCompileSignalCallback>		OnShaderCompiled;

		std::shared_ptr<ShaderConstantParameters>	mConstantParams;
		size_t										mConstantParamsBufferSize;

	};
}
