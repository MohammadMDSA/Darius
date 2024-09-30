#include "Renderer/pch.hpp"
#include "SimpleRayTracingRenderer.hpp"

#include "Renderer/RayTracing/Utils/Utils.hpp"

#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsUtils/RootSignature.hpp>
#include <Graphics/GraphicsUtils/Shader/Shaders.hpp>
#include <Shaders/RayTracing/RaytracingHlslCompat.h>

using namespace D_GRAPHICS_UTILS;
using namespace D_GRAPHICS_SHADERS;

namespace
{
	const wchar_t* c_hitGroupName = L"SimpleHitGroup";

}

namespace Darius::Renderer::RayTracing::Pipeline
{
	SimpleRayTracingPipeline::SimpleRayTracingPipeline() :
		mRTSO(nullptr),
		mInitialized(false),
		mRaygenShaderName("rayGen"_SId),
		mClosestHitShaderName("chs"_SId),
		mMissShaderName("miss"_SId),
		mLibName("SimpleRTLib"_SId)
	{

	}

	void SimpleRayTracingPipeline::Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!mInitialized);

		CreateRaytracingPipelineStateObject();


		mInitialized = true;
	}

	void SimpleRayTracingPipeline::Shutdown()
	{
		D_ASSERT(mInitialized);

		mRTSO.reset();
	}

	void SimpleRayTracingPipeline::CreateRaytracingPipelineStateObject()
	{
		mRTSO = std::make_unique<RayTracingStateObject>();

		// Miss Shader
		std::shared_ptr<RootSignature> missRootSig = std::make_shared<RootSignature>();
		missRootSig->Finalize(L"Simple RayTracing Pipeling Miss Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		std::shared_ptr<MissShader> missShader = std::make_shared<MissShader>(mMissShaderName, mLibName, missRootSig);

		// Closest Hit Shader
		std::shared_ptr<RootSignature> chRootSig = std::make_shared<RootSignature>(1);
		(*chRootSig)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL, 2);
		chRootSig->Finalize(L"Simple RayTracing Pipeling CH Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		std::shared_ptr<ClosestHitShader> chShader = std::make_shared<ClosestHitShader>(mClosestHitShaderName, mLibName, chRootSig);
		RayTracingHitGroup hitGroup;
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		hitGroup.Name = c_hitGroupName;
		hitGroup.ClosestHitShader = chShader;

		// Ray Generation Shader
		std::shared_ptr<RootSignature> rayGenRootSig = std::make_shared<RootSignature>(2);
		(*rayGenRootSig)[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 5, D3D12_SHADER_VISIBILITY_ALL, 1);
		(*rayGenRootSig)[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 5, D3D12_SHADER_VISIBILITY_ALL, 1);
		rayGenRootSig->Finalize(L"Simple RayTracing Pipeling RayGen Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		std::shared_ptr<RayGenerationShader> rayGenShader = std::make_shared<RayGenerationShader>(mRaygenShaderName, mLibName, rayGenRootSig);

		// Adding shader config
		mRTSO->SetShaderConfig<SimplePayload>();

		// Adding hit groupd
		mRTSO->AddHitGroup(hitGroup);

		// Adding miss shader
		mRTSO->AddMissShader(missShader);

		// Adding ray generation shader
		mRTSO->AddRayGenerationShader(rayGenShader);

		// Adding global root signature
		std::shared_ptr<RootSignature> globalRootSig = std::make_shared<RootSignature>(SimpleRayTracing::GlobalRootSignatureBindings::Count, 3);
		SamplerDesc defaultSamplerDesc;
		globalRootSig->InitStaticSampler(10, defaultSamplerDesc); // Default sampler
		globalRootSig->InitStaticSampler(11, defaultSamplerDesc); // Cube map sampler
		globalRootSig->InitStaticSampler(12, D_GRAPHICS::SamplerLinearWrapDesc); // Linear wrap sampler
		(*globalRootSig)[SimpleRayTracing::GlobalRootSignatureBindings::GlobalConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL, 0);
		(*globalRootSig)[SimpleRayTracing::GlobalRootSignatureBindings::GlobalSRVTable].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 5, D3D12_SHADER_VISIBILITY_ALL, 0);
		globalRootSig->Finalize(L"Simple RayTracing Pipeling Globl Root Sig", D3D12_ROOT_SIGNATURE_FLAG_NONE);
		mRTSO->SetGlobalRootSignature(globalRootSig);

		// Adding pipeline config
		mRTSO->SetPipelineConfig(1u);

		// Adding DXIL Libraries
		mRTSO->ResolveDXILLibraries();

		D_ASSERT(mRTSO->GetCurrentIndex() == 11u);

#ifdef _DEBUG
		D_RENDERER_RT_UTILS::PrintStateObjectDesc(mRTSO->GetDesc());
#endif // _DEBUG

		mRTSO->Finalize(L"Simple Ray Traing Pipeline State Object");
	}

}
