#include "Renderer/pch.hpp"
#include "PathTracingPipeline.hpp"

#include "Renderer/RayTracing/Utils/Utils.hpp"

#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsUtils/RootSignature.hpp>
#include <Graphics/GraphicsUtils/Shaders.hpp>
#include <Shaders/RayTracing/RaytracingHlslCompat.h>

using namespace D_GRAPHICS_UTILS;
using namespace D_GRAPHICS_SHADERS;

namespace
{
	const wchar_t* c_hitGroupName = L"DefaultPathTracingHitGroup";
	const wchar_t* c_raygenShaderName = L"MainRenderRayGen";
	const wchar_t* c_closestHitShaderName = L"MainRenderCHS";
	const wchar_t* c_missShaderName = L"MainRenderMiss";
	const wchar_t* c_libName = L"PathTracingLib";

}

namespace Darius::Renderer::RayTracing::Pipeline
{
	PathTracingPipeline::PathTracingPipeline() :
		mRTSO(nullptr),
		mInitialized(false)
	{}

	void PathTracingPipeline::Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!mInitialized);

		CreateRaytracingPipelineStateObject();


		mInitialized = true;
	}

	void PathTracingPipeline::Shutdown()
	{
		D_ASSERT(mInitialized);

		mRTSO.reset();
	}

	void PathTracingPipeline::CreateRaytracingPipelineStateObject()
	{
		mRTSO = std::make_unique<RayTracingStateObject>();

		// Miss Shader
		std::shared_ptr<RootSignature> missRootSig = std::make_shared<RootSignature>();
		missRootSig->Finalize(L"Path Tracing Pipeling Miss Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		std::shared_ptr<MissShader> missShader = std::make_shared<MissShader>(c_missShaderName, c_libName, missRootSig);

		// Closest Hit Shader
		std::shared_ptr<RootSignature> chRootSig = std::make_shared<RootSignature>(5);
		(*chRootSig)[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 2, D3D12_SHADER_VISIBILITY_ALL, 2);				// Mesh Vertices Data
		(*chRootSig)[1].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL, 2);													// Mesh Constants
		(*chRootSig)[2].InitAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_ALL, 2);													// Material Constants
		(*chRootSig)[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_ALL, 2);				// Material Textures
		(*chRootSig)[4].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 10, D3D12_SHADER_VISIBILITY_ALL, 2);			// Material Texture Samplers
		chRootSig->Finalize(L"Path Tracing Pipeling CH Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		std::shared_ptr<ClosestHitShader> chShader = std::make_shared<ClosestHitShader>(c_closestHitShaderName, c_libName, chRootSig);
		RayTracingHitGroup hitGroup;
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		hitGroup.Name = c_hitGroupName;
		hitGroup.ClosestHitShader = chShader;

		// Ray Generation Shader
		std::shared_ptr<RootSignature> rayGenRootSig = std::make_shared<RootSignature>(1);
		(*rayGenRootSig)[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 5, D3D12_SHADER_VISIBILITY_ALL, 1);							// Outputs and Gbuffers
		rayGenRootSig->Finalize(L"Path Tracing Pipeling RayGen Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		std::shared_ptr<RayGenerationShader> rayGenShader = std::make_shared<RayGenerationShader>(c_raygenShaderName, c_libName, rayGenRootSig);

		// Adding shader config
		mRTSO->SetShaderConfig<PathTracerRayPayload>();

		// Adding hit groupd
		mRTSO->AddHitGroup(hitGroup);

		// Adding miss shader
		mRTSO->AddMissShader(missShader);

		// Adding ray generation shader
		mRTSO->AddRayGenerationShader(rayGenShader);

		// Adding global root signature
		std::shared_ptr<RootSignature> globalRootSig = std::make_shared<RootSignature>(PathTracing::GlobalRootSignatureBindings::Count, 3);
		SamplerDesc defaultSamplerDesc;
		globalRootSig->InitStaticSampler(10, defaultSamplerDesc); // Default sampler
		globalRootSig->InitStaticSampler(11, defaultSamplerDesc); // Cube map sampler
		globalRootSig->InitStaticSampler(12, D_GRAPHICS::SamplerLinearWrapDesc); // Linear wrap sampler
		(*globalRootSig)[PathTracing::GlobalRootSignatureBindings::GlobalConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL, 0);
		(*globalRootSig)[PathTracing::GlobalRootSignatureBindings::GlobalRayTracingConstants].InitAsConstantBuffer(1, D3D12_SHADER_VISIBILITY_ALL, 0);
		(*globalRootSig)[PathTracing::GlobalRootSignatureBindings::GlobalSRVTable].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 5, D3D12_SHADER_VISIBILITY_ALL, 0);
		(*globalRootSig)[PathTracing::GlobalRootSignatureBindings::GlobalLightData].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 5, D3D12_SHADER_VISIBILITY_ALL);
		globalRootSig->Finalize(L"Path Tracing Pipeling Globl Root Sig", D3D12_ROOT_SIGNATURE_FLAG_NONE);
		mRTSO->SetGlobalRootSignature(globalRootSig);

		// Adding pipeline config
		mRTSO->SetPipelineConfig(2u);

		// Adding DXIL Libraries
		mRTSO->ResolveDXILLibraries();

		D_ASSERT(mRTSO->GetCurrentIndex() == 11u);

#ifdef _DEBUG
		D_RENDERER_RT_UTILS::PrintStateObjectDesc(mRTSO->GetDesc());
#endif // _DEBUG

		mRTSO->Finalize(L"Simple Ray Traing Pipeline State Object");
	}

}
