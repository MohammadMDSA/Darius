#pragma once

#include "Shaders.hpp"

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils
{
	class StateObject
	{
	public:
		StateObject(D3D12_STATE_OBJECT_TYPE type) :
			mType(type),
			mStateObject(nullptr),
			mFinalized(false)
		{}

		virtual D3D12_STATE_OBJECT_DESC const*	GetDesc() = 0;

		void									Finalize(std::wstring const& name);

		INLINE ID3D12StateObject*				GetStateObject() const { return mStateObject; }

		static void								DestroyAll();

	protected:
		virtual void							CleanUp() = 0;

	private:
		D3D12_STATE_OBJECT_TYPE					mType;
		ID3D12StateObject*						mStateObject;
		bool									mFinalized;
	};

	class RayTracingStateObject : public StateObject
	{
	public:
		INLINE RayTracingStateObject() :
			StateObject(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE),
			mPipelineDesc(D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE),
			mMaxAttributeSizeInBytes(0u),
			mMaxPayloadSizeInBytes(0u),
			mMaxTraceRecursionDepth(1u),
			mGlobalRootSignature(nullptr),
			mMaxLocalRootSignatureSize(0u),
			mCurrentIndex(0u)
		{
		}

		template<typename PAYLOAD, typename ATTRIBUTE = DirectX::XMFLOAT2>
		INLINE void								SetShaderConfig()
		{
			mMaxAttributeSizeInBytes = sizeof(ATTRIBUTE);
			mMaxPayloadSizeInBytes = sizeof(PAYLOAD);
			mPipelineDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>()->Config(mMaxPayloadSizeInBytes, mMaxAttributeSizeInBytes);

			mCurrentIndex++; // For shader config
		}

		INLINE void								SetPipelineConfig(UINT maxTraceRecursionDepth)
		{
			mPipelineDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>()->Config(maxTraceRecursionDepth);

			mCurrentIndex++; // For pipeline config 
		}

		INLINE void								SetGlobalRootSignature(ID3D12RootSignature* globalRT)
		{
			mPipelineDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>()->SetRootSignature(globalRT);

			mCurrentIndex++; // For global root signature
		}

		void									AddMissShader(std::shared_ptr<Shaders::MissShader> missShader);
		void									AddRayGenerationShader(std::shared_ptr < Shaders::RayGenerationShader> rayGenerationShader);
		void									AddHitGroup(Shaders::RayTracingHitGroup const& hitGroup, bool isDefault = false);

		void									ResolveDXILLibraries();

		INLINE virtual D3D12_STATE_OBJECT_DESC const* GetDesc() override
		{
			return mPipelineDesc;
		}

		// Getters
		UINT									GetMaxAttributeSizeInBytes() const { return mMaxAttributeSizeInBytes; }
		UINT									GetMaxPayloadSizeInBytes() const { return mMaxPayloadSizeInBytes; }
		UINT									GetMaxTraceRecursionDepth() const { return mMaxTraceRecursionDepth; }
		ID3D12RootSignature*					GetGlobalRootSignature() const { return mGlobalRootSignature; }
		UINT									GetCurrentIndex() const { return mCurrentIndex; }

		INLINE D_CONTAINERS::DVector<std::shared_ptr<Shaders::RayGenerationShader>> const& GetRayGenerationShaders() const { return mRayGenerationShaders; }
		INLINE D_CONTAINERS::DVector<std::shared_ptr<Shaders::MissShader>> const& GetMissShaders() const { return mMissShaders; }
		INLINE D_CONTAINERS::DVector<Shaders::RayTracingHitGroup> const& GetHitGroups() const { return mHitGroups; }

	protected:
		virtual void							CleanUp() override;

	private:

		void									ProcessShader(Shaders::RayTracingShader* shader);
		CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* FindOrCreateRootSignatureSubObject(ID3D12RootSignature* rootSignature);
		CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* FindExistingRootSignatureSubObject(ID3D12RootSignature* rootSignature) const;

		// Shader Config
		UINT									mMaxAttributeSizeInBytes;
		UINT									mMaxPayloadSizeInBytes;

		// Pipeline Config
		UINT									mMaxTraceRecursionDepth;

		ID3D12RootSignature*					mGlobalRootSignature;

		UINT									mCurrentIndex;
		UINT									mMaxLocalRootSignatureSize;

		CD3DX12_STATE_OBJECT_DESC				mPipelineDesc;

		D_CONTAINERS::DMap<ID3D12RootSignature*, CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT*> mRootSignatureSubObjectMap;
		D_CONTAINERS::DMap<std::wstring, D_CONTAINERS::DVector<std::wstring>> mLibraryExportNamesMap;
		D_CONTAINERS::DSet<std::shared_ptr<D3D12_SHADER_BYTECODE>> mShaderByteCodes;

		D_CONTAINERS::DVector<std::shared_ptr<Shaders::RayGenerationShader>> mRayGenerationShaders;
		D_CONTAINERS::DVector<std::shared_ptr<Shaders::MissShader>> mMissShaders;
		D_CONTAINERS::DVector<Shaders::RayTracingHitGroup> mHitGroups;
	};
}
