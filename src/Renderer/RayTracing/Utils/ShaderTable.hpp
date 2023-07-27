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
			UINT		MaxViewDescriptorsPerRecord = 0;
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
			mTableData(0)
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
		void SetCallableShaderParameters(UINT recordIndex, UINT inOffsetWithinRootSignature, const T& parameters)
		{
			const UINT shaderTableOffset = mCallableShaderTableOffset;
			WriteLocalShaderRecord(shaderTableOffset, recordIndex, ShaderIdentifierSize + inOffsetWithinRootSignature, &parameters, sizeof(parameters));
		}

		void SetLocalShaderParameters(UINT shaderTableOffset, UINT recordIndex, UINT inOffsetWithinRootSignature, const void* data, UINT dataSize);

		void CopyLocalShaderParameters(UINT inshaderTableOffset, UINT inDestrecordIndex, UINT inSourcerecordIndex, UINT inOffsetWithinRootSignature);

		void CopyHitGroupParameters(UINT inDestrecordIndex, UINT inSourcerecordIndex, UINT inOffsetWithinRootSignature);

		void SetRayGenIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetMissIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetCallableIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetDefaultHitGroupIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetHitGroupIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetRayGenIdentifiers(D_CONTAINERS::DVector<D_GRAPHICS_SHADERS::ShaderIdentifier> const& identifiers);

		void SetDefaultMissshaderIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void SetDefaultCallableshaderIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier);

		void CopyToGpu(Darius::Renderer::RayTracing::RayTracingCommandContext& context);

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
