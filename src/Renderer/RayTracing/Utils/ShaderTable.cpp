#include "Renderer/pch.hpp"
#include "ShaderTable.hpp"

#include "Renderer/RayTracing/RayTracingCommandContext.hpp"

#include <Core/Memory/Memory.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>

using namespace D_GRAPHICS_SHADERS;

namespace Darius::Renderer::RayTracing::Utils
{
	void ShaderTable::Init(Initializer const& initializer, ID3D12Device5* device, D_CONTAINERS::DVector<D_GRAPHICS_SHADERS::ShaderIdentifier> const& rayGenIdentifiers, D_GRAPHICS_SHADERS::ShaderIdentifier const& defaultHitGroupIdentifier)
	{
		D_ASSERT_M(initializer.LocalRootDataSize <= 4098, "The maximum size of a local root signature is 4KB.");
		D_ASSERT_M(initializer.NumRayGenShaders >= 1, "All shader tables must contain at least on raygen shader");

		mLocalRecordSizeUnaligned = ShaderIdentifierSize + initializer.LocalRootDataSize;
		mLocalRecordStride = D_MEMORY::AlignUp(mLocalRecordSizeUnaligned, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

		mNumRayGenShaders = initializer.NumRayGenShaders;
		mNumMissRecords = initializer.NumMissRecords;
		mNumHitRecords = initializer.NumHitRecords;
		mNumCallableRecords = initializer.NumCallableRecords;

		UINT totalDataSize = 0u;

		mRayGenShaderTableOffset = totalDataSize;
		totalDataSize += mNumRayGenShaders * RayGenRecordStride;
		totalDataSize = D_MEMORY::AlignUp(totalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		mDefaultHitGroupShaderTableOffset = totalDataSize;
		totalDataSize += ShaderIdentifierSize;
		totalDataSize = D_MEMORY::AlignUp(totalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		mHitGroupShaderTableOffset = totalDataSize;
		totalDataSize += initializer.NumHitRecords * mLocalRecordStride;
		totalDataSize = D_MEMORY::AlignUp(totalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		
		mCallableShaderTableOffset = totalDataSize;
		totalDataSize += initializer.NumCallableRecords * mLocalRecordStride;
		totalDataSize = D_MEMORY::AlignUp(totalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		mMissShaderTableOffset = totalDataSize;
		totalDataSize += initializer.NumMissRecords * mLocalRecordStride;
		totalDataSize = D_MEMORY::AlignUp(totalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		mTableData.reserve(totalDataSize);
		ZeroMemory(mTableData.data(), totalDataSize);

		mDefaultMissShaderSet = false;
	}

	void ShaderTable::WriteData(UINT offset, void const* data, UINT dataSize)
	{
		D_ASSERT_M(mTableData.size() > 0, "Shader table must be initialized before ant writing to it");

		std::memcpy(mTableData.data() + offset, data, dataSize);

		mIsDirty = true;
	}

	void ShaderTable::WriteLocalShaderRecord(UINT shaderTableOffset, UINT recordIndex, UINT offsetWithinRecord, void const* data, UINT dataSize)
	{
		D_ASSERT_M(offsetWithinRecord % 4 == 0, "SBT record parameters must be written on DWORD-aligned boundary");
		D_ASSERT_M(dataSize % 4 == 0, TEXT("SBT record parameters must be DWORD-aligned"));
		D_ASSERT_M(offsetWithinRecord + dataSize <= mLocalRecordSizeUnaligned, "SBT record write request is out of bounds");

		const UINT writeOffset = shaderTableOffset + mLocalRecordStride * recordIndex + offsetWithinRecord;

		WriteData(writeOffset, data, dataSize);

	}


	void ShaderTable::SetLocalShaderParameters(UINT shaderTableOffset, UINT recordIndex, UINT inOffsetWithinRootSignature, const void* data, UINT dataSize)
	{
		WriteLocalShaderRecord(shaderTableOffset, recordIndex, ShaderIdentifierSize + inOffsetWithinRootSignature, data, dataSize);
	}

	void ShaderTable::CopyLocalShaderParameters(UINT inshaderTableOffset, UINT inDestrecordIndex, UINT inSourcerecordIndex, UINT inOffsetWithinRootSignature)
	{
		const UINT baseOffset = inshaderTableOffset + ShaderIdentifierSize + inOffsetWithinRootSignature;
		const UINT destOffset = baseOffset + mLocalRecordStride * inDestrecordIndex;
		const UINT sourceOffset = baseOffset + mLocalRecordStride * inSourcerecordIndex;
		const UINT copySize = mLocalRecordStride - ShaderIdentifierSize - inOffsetWithinRootSignature;
		D_ASSERT(copySize <= mLocalRecordStride);

		std::memcpy(mTableData.data() + destOffset, mTableData.data() + sourceOffset, copySize);

		mIsDirty = true;
	}

	void ShaderTable::CopyHitGroupParameters(UINT inDestrecordIndex, UINT inSourcerecordIndex, UINT inOffsetWithinRootSignature)
	{
		const UINT shaderTableOffset = mHitGroupShaderTableOffset;
		CopyLocalShaderParameters(shaderTableOffset, inDestrecordIndex, inSourcerecordIndex, inOffsetWithinRootSignature);
	}

	void ShaderTable::SetRayGenIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier)
	{
		const UINT writeOffset = mRayGenShaderTableOffset + recordIndex * RayGenRecordStride;
		WriteData(writeOffset, shaderIdentifier.Data, ShaderIdentifierSize);
	}

	void ShaderTable::SetMissIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier)
	{
		const UINT writeOffset = mMissShaderTableOffset + recordIndex * mLocalRecordStride;

		if (recordIndex == 0)
		{
			mDefaultMissShaderSet = true;
		}
		WriteData(writeOffset, shaderIdentifier.Data, ShaderIdentifierSize);
	}

	void ShaderTable::SetCallableIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier)
	{
		const UINT writeOffset = mCallableShaderTableOffset + recordIndex * mLocalRecordStride;
		WriteData(writeOffset, shaderIdentifier.Data, ShaderIdentifierSize);
	}

	void ShaderTable::SetDefaultHitGroupIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier)
	{
		const UINT writeOffset = mDefaultHitGroupShaderTableOffset;
		WriteData(writeOffset, shaderIdentifier.Data, ShaderIdentifierSize);
	}

	void ShaderTable::SetHitGroupIdentifier(UINT recordIndex, D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier)
	{
		D_ASSERT_M(shaderIdentifier.IsValid(), "Shader identifier must be initialized FD3D12RayTracingPipelineState::GetshaderIdentifier() before use.");
		D_ASSERT(sizeof(shaderIdentifier.Data) >= ShaderIdentifierSize);

		const UINT writeOffset = mHitGroupShaderTableOffset + recordIndex * mLocalRecordStride;
		WriteData(writeOffset, shaderIdentifier.Data, ShaderIdentifierSize);
	}

	void ShaderTable::SetRayGenIdentifiers(D_CONTAINERS::DVector<D_GRAPHICS_SHADERS::ShaderIdentifier> const& identifiers)
	{
		D_ASSERT((UINT)identifiers.size() == mNumRayGenShaders);
		for (UINT Index = 0; Index < identifiers.size(); ++Index)
		{
			SetRayGenIdentifier(Index, identifiers[Index]);
		}
	}

	void ShaderTable::SetDefaultMissshaderIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier)
	{
		// Set all slots to the same default
		for (UINT Index = 0; Index < mNumMissRecords; ++Index)
		{
			SetMissIdentifier(Index, shaderIdentifier);
		}

		mDefaultMissShaderSet = false;
	}

	void ShaderTable::SetDefaultCallableshaderIdentifier(D_GRAPHICS_SHADERS::ShaderIdentifier const& shaderIdentifier)
	{
		for (UINT Index = 0; Index < mNumCallableRecords; ++Index)
		{
			SetCallableIdentifier(Index, shaderIdentifier);
		}
	}

	void ShaderTable::CopyToGpu(RayTracingCommandContext& context)
	{
		D_PROFILING::ScopedTimer _prof(L"Shader Table Buffer Creation");

		D_ASSERT_M(mTableData.size(), "Shader table is expected to be initialized before copying to GPU.");

		mTableBufferResource.Create(L"Shader Table", 1, (UINT)mTableData.size(), mTableData.data(), D3D12_RESOURCE_STATE_COPY_DEST);

		context.TransitionResource(mTableBufferResource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
	}
}
