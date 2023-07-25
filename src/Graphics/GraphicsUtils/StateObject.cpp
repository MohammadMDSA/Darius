#include "Graphics/pch.hpp"
#include "StateObject.hpp"

#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

#include <Core/Hash.hpp>

#include <mutex>

using Microsoft::WRL::ComPtr;

static std::map<size_t, ComPtr<ID3D12StateObject>> s_StateObjectHashMap;

namespace Darius::Graphics::Utils
{
	void StateObject::DestroyAll()
	{
		s_StateObjectHashMap.clear();
	}

	void StateObject::Finalize(std::wstring const& name)
	{

		if (mFinalized)
			return;

		auto desc = GetDesc();

		D_ASSERT(desc->Type == mType);

		// Creating hash
		size_t hashCode = D_CORE::HashState(&(desc->Type));
		hashCode = D_CORE::HashState(desc->pSubobjects, desc->NumSubobjects, hashCode);

		ID3D12StateObject** SORef = nullptr;
		bool firstCompile = false;
		{
			static std::mutex s_HashMapMutex;
			auto iter = s_StateObjectHashMap.find(hashCode);

			// Reserve space so the next inquiry will find that someone got herer first.
			if (iter == s_StateObjectHashMap.end())
			{
				SORef = s_StateObjectHashMap[hashCode].GetAddressOf();
				firstCompile = true;
			}
			else
				SORef = iter->second.GetAddressOf();

			if (firstCompile)
			{
				D_HR_CHECK(D_GRAPHICS_DEVICE::GetDevice5()->CreateStateObject(desc, IID_PPV_ARGS(&mStateObject)));

				mStateObject->SetName(name.c_str());

				s_StateObjectHashMap[hashCode].Attach(mStateObject);
				D_ASSERT(*SORef == mStateObject);
			}
			else
			{
				while (*SORef == nullptr)
					std::this_thread::yield();
				mStateObject = *SORef;
			}

			mFinalized = true;
		}

		CleanUp();
	}

	void RayTracingStateObject::AddMissShader(Shaders::MissShader* missShader)
	{
		ProcessShader(missShader);
	}

	void RayTracingStateObject::AddRayGenerationShader(Shaders::RayGenerationShader* rayGenerationShader)
	{
		ProcessShader(rayGenerationShader);
	}

	void RayTracingStateObject::AddHitGroup(Shaders::RayTracingHitGroup const& hitGroup)
	{
		// Add hit group sub object
		auto hitGroupSubObj = mPipelineDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroupSubObj->SetHitGroupType(hitGroup.Type);

		hitGroupSubObj->SetHitGroupExport(hitGroup.Name.c_str());

		mCurrentIndex++; // For hit group

		if (hitGroup.ClosestHitShader)
		{
			hitGroupSubObj->SetClosestHitShaderImport(hitGroup.ClosestHitShader->GetName().c_str());
			ProcessShader(hitGroup.ClosestHitShader);
		}

		if (hitGroup.IntersectionShader)
		{
			hitGroupSubObj->SetIntersectionShaderImport(hitGroup.IntersectionShader->GetName().c_str());
			ProcessShader(hitGroup.IntersectionShader);
		}

		if (hitGroup.AnyHitShader)
		{
			hitGroupSubObj->SetAnyHitShaderImport(hitGroup.AnyHitShader->GetName().c_str());
			ProcessShader(hitGroup.AnyHitShader);
		}

	}


	void RayTracingStateObject::ProcessShader(Shaders::RayTracingShader* shader)
	{
		auto localRootSig = shader->GetLocalRootSignature()->GetSignature();
		mMaxLocalRootSignatureSize = std::max(mMaxLocalRootSignatureSize, shader->GetRootParametersSizeInBytes());

		// Adding the shader name to associated library to create the lib later
		mLibraryExportNamesMap[shader->GetLibraryName()].push_back(shader->GetName());

		// TODO: Couple root signature associations
		// Adding the shader to its local root signature association
		auto localRootSignatureSubObject = FindOrCreateRootSignatureSubObject(localRootSig);

		auto assocSubObj = mPipelineDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		assocSubObj->SetSubobjectToAssociate(*localRootSignatureSubObject);
		assocSubObj->AddExport(shader->GetName().c_str());

		mCurrentIndex++; // For association
	}

	CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* RayTracingStateObject::FindOrCreateRootSignatureSubObject(ID3D12RootSignature* rootSignature)
	{
		auto result = FindExistingRootSignatureSubObject(rootSignature);

		if (result == nullptr)
		{
			result = mPipelineDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
			result->SetRootSignature(rootSignature);

			mRootSignatureSubObjectMap.emplace(rootSignature, result);
			
			mCurrentIndex++; // For local root signature sub object
		}

		return result;
	}

	CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT* RayTracingStateObject::FindExistingRootSignatureSubObject(ID3D12RootSignature* rootSignature) const
	{
		if (mRootSignatureSubObjectMap.contains(rootSignature))
			return mRootSignatureSubObjectMap.at(rootSignature);
		return nullptr;
	}

	void RayTracingStateObject::ResolveDXILLibraries()
	{
		// Adding shader libraries
		for (auto const& [libName, exportNames] : mLibraryExportNamesMap)
		{
			auto libSubObj = mPipelineDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

			// Setting library shader
			auto shader = D_GRAPHICS::GetShaderByName(WSTR2STR(libName));
			std::shared_ptr<D3D12_SHADER_BYTECODE> libdxil = std::make_shared<D3D12_SHADER_BYTECODE>(shader->GetBufferPointer(), shader->GetBufferSize());
			mShaderByteCodes.insert(libdxil);
			libSubObj->SetDXILLibrary(libdxil.get());

			// Adding export names
			for (auto const& exportName : exportNames)
			{
				libSubObj->DefineExport(exportName.c_str());
			}

			mCurrentIndex++; // For the dxil lilb
		}
	}

	void RayTracingStateObject::CleanUp()
	{
		// Clearing maps
		mRootSignatureSubObjectMap.clear();
		mLibraryExportNamesMap.clear();
		mShaderByteCodes.clear();

		// Removing all existing data
		mPipelineDesc = {};
	}
}