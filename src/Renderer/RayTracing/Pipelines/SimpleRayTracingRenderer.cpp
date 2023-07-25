#include "Renderer/pch.hpp"
#include "SimpleRayTracingRenderer.hpp"

#include "Renderer/RayTracing/Utils/Utils.hpp"

#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsUtils/RootSignature.hpp>
#include <Graphics/GraphicsUtils/Shaders.hpp>
#include <Shaders/RayTracing/Rendering/Simple/RaytracingHlslCompat.h>

using namespace D_GRAPHICS_UTILS;
using namespace D_GRAPHICS_SHADERS;

namespace
{
	const wchar_t* c_hitGroupName = L"SimpleHitGroup";
	const wchar_t* c_raygenShaderName = L"rayGen";
	const wchar_t* c_closestHitShaderName = L"chs";
	const wchar_t* c_missShaderName = L"miss";
	const wchar_t* c_libName = L"SimpleRTLib";

}

namespace Darius::Renderer::RayTracing::Pipeline
{
	SimpleRayTracingPipeline::SimpleRayTracingPipeline() :
		mInitialized(false),
		mRTSO(nullptr)
	{}

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

	void SimpleRayTracingPipeline::Update()
	{

	}

	void SimpleRayTracingPipeline::Render(std::wstring const& jobId, RayTracingCommandContext& context)
	{

	}

	void SimpleRayTracingPipeline::CreateRaytracingPipelineStateObject()
	{
		mRTSO = std::make_unique<RayTracingStateObject>();

		// Miss Shader
		std::shared_ptr<RootSignature> missRootSig = std::make_shared<RootSignature>();
		missRootSig->Finalize(L"Simple RayTracing Pipeling Miss Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		MissShader missShader(c_missShaderName, c_libName, missRootSig);

		// Closest Hit Shader
		std::shared_ptr<RootSignature> chRootSig = std::make_shared<RootSignature>(1);
		(*chRootSig)[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL, 0);
		chRootSig->Finalize(L"Simple RayTracing Pipeling CH Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		ClosestHitShader chShader(c_closestHitShaderName, c_libName, chRootSig);
		RayTracingHitGroup hitGroup = { D3D12_HIT_GROUP_TYPE_TRIANGLES, c_hitGroupName, &chShader};

		// Ray Generation Shader
		std::shared_ptr<RootSignature> rayGenRootSig = std::make_shared<RootSignature>(1);
		(*rayGenRootSig)[0].InitAsDescriptorTable(2);
		(*rayGenRootSig)[0].SetTableRange(0, D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1, 0);
		(*rayGenRootSig)[0].SetTableRange(1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, 0);
		rayGenRootSig->Finalize(L"Simple RayTracing Pipeling RayGen Root Sig", D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
		RayGenerationShader rayGenShader(c_raygenShaderName, c_libName, rayGenRootSig);

		// Adding shader config
		mRTSO->SetShaderConfig<Payload>();

		// Adding hit groupd
		mRTSO->AddHitGroup(hitGroup);

		// Adding miss shader
		mRTSO->AddMissShader(&missShader);

		// Adding ray generation shader
		mRTSO->AddRayGenerationShader(&rayGenShader);

		// Adding global root signature
		RootSignature globalRootSig;
		globalRootSig.Finalize(L"Simple RayTracing Pipeling Globl Root Sig", D3D12_ROOT_SIGNATURE_FLAG_NONE);
		mRTSO->SetGlobalRootSignature(globalRootSig.GetSignature());

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
