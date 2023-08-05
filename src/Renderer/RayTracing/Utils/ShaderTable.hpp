#pragma once

#include <Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Shaders.hpp>

#ifndef D_RENDERER_RT_UTILS
#define D_RENDERER_RT_UTILS Darius::Renderer::RayTracing::Utils
#endif // !D_RENDERER_RT_UTILS

namespace Darius::Renderer::RayTracing
{
	class RayTracingCommandContext;
}

namespace Darius::Renderer::RayTracing::Utils
{
	struct HitGroupSystemParameters
	{
		//D3D12_GPU_VIRTUAL_ADDRESS IndexBuffer;
		//D3D12_GPU_VIRTUAL_ADDRESS VertexBuffer;
	};

	struct RayGenerationSystemParameters
	{
		D3D12_GPU_VIRTUAL_ADDRESS	AccelerationStructureBuffer;
	};

	class ShaderTable
	{
	public:
		struct Initializer
		{
			UINT		NumRayGenShaders = 0;
			UINT		NumMissShaders = 0;
			UINT		NumMissRecords = 0;
			UINT		NumHitRecords = 0;
			UINT		NumCallableRecords = 0;
			UINT		LocalRootDataSize = 0;
		};

		ShaderTable() :
			mLocalRecordSizeUnaligned(0u),
			mNumHitRecords(0u),
			mNumRayGenShaders(0u),
			mNumCallableRecords(0u),
			mNumMissRecords(0u),
			mNumLocalRecords(0u),
			mRayGenShaderTableOffset(0u),
			mMissShaderTableOffset(0u),
			mDefaultHitGroupShaderTableOffset(0u),
			mHitGroupShaderTableOffset(0u),
			mCallableShaderTableOffset(0u),
			mDefaultMissShaderSet(0u),
			mTableData(0),
			mTableBufferResource(D3D12_RESOURCE_FLAG_NONE)
		{
		}

		~ShaderTable()
		{
		}

		void Init(Initializer const& initializer, ID3D12Device5* device, D_CONTAINERS::DVector<D_GRAPHICS_SHADERS::ShaderIdentifier> const& rayGenIdentifiers, D_GRAPHICS_SHADERS::ShaderIdentifier const& defaultHitGroupIdentifier);
		
		INLINE bool IsDefaultMissShaderSet() const { return mDefaultMissShaderSet; }

		template <typename T>
		void SetLocalShaderParameters(UINT shaderTableOffset, UINT recordIndex, UINT inOffsetWithinRootSignature, const T& parameters)
		{
			WriteLocalShaderRecord(shaderTableOffset, recordIndex, ShaderIdentifierSize + inOffsetWithinRootSignature, &parameters, sizeof(parameters));
		}

		template <typename T>
		void SetMissShaderParameters(UINT recordIndex, UINT inOffsetWithinRootSignature, const T& parameters)
		{
			const UINT shaderTableOffset = mMissShaderTableOffset;
			WriteLocalShaderRecord(shaderTableOffset, recordIndex, ShaderIdentifierSize + inOffsetWithinRootSignature, &parameters, sizeof(parameters));
		}

		template <typename T>
		void SetHitGroupParameters(UINT recordIndex, UINT inOffsetWithingRootSignature, T const& parameters)
		{
			const UINT shaderTableOffset = mHitGroupShaderTableOffset;
			WriteLocalShaderRecord(shaderTableOffset, recordIndex, ShaderIdentifierSize + inOffsetWithingRootSignature, &parameters, sizeof(parameters));
		}

		template <typename T>
		void SetRayGenerationShaderParameters(UINT recordIndex, UINT offsetWithinRootSignature, T const& parameters)
		{
			const UINT shaderTableOffset = mRayGenShaderTableOffset;
			WriteLocalShaderRecord(mRayGenShaderTableOffset, recordIndex, ShaderIdentifierSize + offsetWithinRootSignature, &parameters, sizeof(parameters));
		}

		template <typename T>
		void SetCallableShaderParameters(UINT recordIndex, UINT inOffsetWithinRootSignature, const T& parameters)
		{
			const UINT shaderTableOffset = mCallableShaderTableOffset;
			WriteLocalShaderRecord(shaderTableOffset, recordIndex, ShaderIdentifierSize + inOffsetWithinRootSignature, &parameters, sizeof(parameters));
		}

		void SetLocalShaderParameters(UINT shaderTableOffset, UINT recordIndex, UINT inOffsetWithinRootSignature, const void* data, UINT dataSize);

		void CopyLocalShaderParameters(UINT inshaderTableOffset, UINT inDestrecordIndex, UINT inSourcerecordIndex, UINT inOffsetWithinRootSignature);

		void CopyHitGroupParameters(UINT recordIndex, UINT inSourcerecordIndex, UINT inOffsetWithinRootSignature);

		void SetRayGenIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetMissIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetCallableIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetDefaultHitGroupIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetHitGroupIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetHitGroupSystemParameters(UINT recordIndex, HitGroupSystemParameters const& params);

		void SetRayGenIdentifiers(D_CONTAINERS::DVector<D_GRAPHICS_SHADERS::ShaderIdentifier> const& identifiers);

		void SetRayGenSystemParameters(UINT recordIndex, RayGenerationSystemParameters const& params);

		void SetDefaultMissShaderIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetDefaultCallableShaderIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void CopyToGpu(Darius::Renderer::RayTracing::RayTracingCommandContext& context);

		INLINE  D3D12_GPU_VIRTUAL_ADDRESS GetShaderTableAddress() const
		{
			D_ASSERT_M(!mIsDirty, "Shader table update is pending, therefore GPU address is not available. Use CopyToGPU() to upload data and acquire a valid GPU buffer address.");
			return mTableBufferResource.GetGpuVirtualAddress();
		}


		D3D12_DISPATCH_RAYS_DESC GetDispatchRaysDesc(UINT rayGenShaderIndex, bool allowHitGroupIndexing) const;

		static constexpr UINT			ShaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		static constexpr UINT			RayGenRecordStride = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	private:
		void							WriteData(UINT offset, void const* data, UINT dataSize);
		void							WriteLocalShaderRecord(UINT shaderTableOffset, UINT recordIndex, UINT offsetWithinRecord, void const* data, UINT dataSize);

	private:

		UINT							mLocalRecordSizeUnaligned;
		UINT							mLocalRecordStride;

		UINT							mNumHitRecords;
		UINT							mNumRayGenShaders;
		UINT							mNumCallableRecords;
		UINT							mNumMissRecords;
		UINT							mNumLocalRecords;

		UINT							mRayGenShaderTableOffset;
		UINT							mMissShaderTableOffset;
		UINT							mDefaultHitGroupShaderTableOffset;
		UINT							mHitGroupShaderTableOffset;
		UINT							mCallableShaderTableOffset;

		D_CONTAINERS::DVector<UINT8>	mTableData;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer mTableBufferResource;

		bool							mDefaultMissShaderSet;
		bool							mIsDirty;

	};

}
