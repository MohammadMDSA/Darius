#include "Renderer/pch.hpp"
#include "Renderer.hpp"

#include "Pipelines/PathTracingPipeline.hpp"
#include "RayTracingScene.hpp"
#include "Renderer/Components/BillboardRendererComponent.hpp"
#include "Renderer/Components/MeshRendererComponent.hpp"
#include "Renderer/Components/SkeletalMeshRendererComponent.hpp"
#include "Renderer/Components/TerrainRendererComponent.hpp"
#include "Renderer/RayTracing/Light/RayTracingLightContext.hpp"

#include <Core/Uuid.hpp>
#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/Texture.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Job/Job.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Renderer/RendererManager.hpp>
#include <ResourceManager/ResourceManager.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS StaticBLASBuildFlags =
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;

using namespace D_CONTAINERS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_MEMORY;
using namespace D_RENDERER_RT_PIPELINE;
using namespace D_RENDERER_RT_UTILS;

namespace Darius::Renderer::RayTracing
{

	// Settings
	UINT													MaxNumBottomLevelAS;

	// Scene
	std::unique_ptr<RayTracingScene>						RTScene;

	// Pipelines
	std::unique_ptr<Pipeline::PathTracingPipeline>			MainRenderPipeline;

	// Heaps
	DescriptorHeap											TextureHeap;
	DescriptorHeap											SamplerHeap;

	// Internal
	bool													_initialized;
	DescriptorHandle										PathTracingCommonSRVs[D_GRAPHICS_DEVICE::gNumFrameResources]; // 0: TLAS, 1: RadIBL, 2: IrradIBL
	DescriptorHandle										PathTracingLightDataSRVs[D_GRAPHICS_DEVICE::gNumFrameResources]; // 0: LighStatus, 1: LightData
	DescriptorHandle										RayGenUAVs[D_GRAPHICS_DEVICE::gNumFrameResources]; // 0: Color Render target
	D_RESOURCE::ResourceRef<TextureResource>				BlackCubeTextureRes({ L"Rasterization Renderer" });

	std::unique_ptr<D_RENDERER_RT_LIGHT::RayTracingLightContext> LightContext;

	ALIGN_DECL_256 struct MeshConstants
	{
		DirectX::XMFLOAT4		colors[3];
	};

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Loading Settings
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("RayTracing.MaxNumBottomLevelAS", MaxNumBottomLevelAS, 100000);

		TextureHeap.Create(L"Ray Tracing SRV, UAV, CBV  Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 16384);
		SamplerHeap.Create(L"Ray Tracing Sampler  Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1024);

		RTScene = std::make_unique<RayTracingScene>(MaxNumBottomLevelAS, (UINT)RayTypes::Count);

		MainRenderPipeline = std::make_unique<Pipeline::PathTracingPipeline>();
		MainRenderPipeline->Initialize(settings);

		for (int i = 0; i < D_GRAPHICS_DEVICE::gNumFrameResources; i++)
		{
			PathTracingCommonSRVs[i] = TextureHeap.Alloc(3);
			PathTracingLightDataSRVs[i] = TextureHeap.Alloc(3);
			RayGenUAVs[i] = TextureHeap.Alloc(3);
		}
		
		BlackCubeTextureRes = D_RESOURCE::GetResource<TextureResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::TextureCubeMapBlack));

		LightContext = std::make_unique<D_RENDERER_RT_LIGHT::RayTracingLightContext>();

	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		LightContext.reset();

		MainRenderPipeline->Shutdown();

		RTScene.reset();
		MainRenderPipeline.reset();

		TextureHeap.Destroy();
		SamplerHeap.Destroy();
	}

	void UpdateRendererComponents(D_GRAPHICS::CommandContext& context)
	{
		D_PROFILING::ScopedTimer _prof(L"Update Renderer Components", context);

		D_CAMERA_MANAGER::Update();

		D_WORLD::IterateComponents<MeshRendererComponent>([&](D_RENDERER::MeshRendererComponent& meshComp)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						meshComp.Update(-1);

					});
			}
		);

		D_WORLD::IterateComponents<SkeletalMeshRendererComponent>([&](D_RENDERER::SkeletalMeshRendererComponent& meshComp)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						meshComp.Update(-1);

					});
			}
		);

		D_WORLD::IterateComponents<BillboardRendererComponent>([&](D_RENDERER::BillboardRendererComponent& meshComp)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						meshComp.Update(-1);

					});
			}
		);

		D_WORLD::IterateComponents<TerrainRendererComponent>([&](D_RENDERER::TerrainRendererComponent& meshComp)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						meshComp.Update(-1);

					});
			}
		);

		if (D_JOB::IsMainThread())
			D_JOB::WaitForThreadsToFinish();
		/*{
			D_PROFILING::ScopedTimer lightProfiling(L"Update Lights");
			D_LIGHT_RAST::Update();
		}*/
	}

	void Update(D_GRAPHICS::CommandContext& context)
	{
		RTScene->Reset();

		// Updating Components
		UpdateRendererComponents(context);

		// Updating BLASes
		{
			D_PROFILING::ScopedTimer _prof(L"Update Blas Instances", context);

			auto createStaticMeshBlas = [scene = RTScene.get()](D_RENDERER_GEOMETRY::Mesh const& mesh, D_CORE::Uuid const& uuid)
			{
				// TODO: Add translucent objects
				BottomLevelAccelerationStructureGeometry BLASGeom = { mesh, uuid, D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE };
				RTScene->AddBottomLevelAS(StaticBLASBuildFlags, BLASGeom, false, false);
			};

			D_WORLD::IterateComponents<MeshRendererComponent>([scene = RTScene.get(), createBlasFunc = createStaticMeshBlas](D_RENDERER::MeshRendererComponent& comp)
				{
					auto const* meshRes = comp.GetMesh();

					if (!meshRes)
						return;

					auto const& mats = comp.GetMaterials();

					auto uuid = meshRes->GetUuid();
					auto const* blas = scene->GetBottomLevelAS(uuid);

					// Create blas if it's not there
					if (!blas)
					{
						createBlasFunc(*meshRes->GetMeshData(), uuid);
					}

					// Submit BLAS instance with all associated data
					scene->AddBottomLevelASInstance(uuid, mats, comp.GetConstantsAddress(), comp.GetTransform()->GetWorld());
				});

		}
	}

	void CreateGlobalBindings(RayTracingCommandContext& context, SceneRenderContext& renderContext, ShaderTable* shaderTable)
	{
		// System bindings
		context.SetPipelineState(*MainRenderPipeline->GetStateObject());	context.SetRootSignature(*MainRenderPipeline->GetStateObject()->GetGlobalRootSignature());
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SamplerHeap.GetHeapPointer());

		// Resource bindings
		context.SetDynamicConstantBufferView(PathTracing::GlobalRootSignatureBindings::GlobalConstants, sizeof(renderContext.Globals), &renderContext.Globals);

		ALIGN_DECL_256 struct GlobalRTConstants
		{
			UINT		MaxRadianceRayRecursionDepth = 2u;
		} RTCB;

		context.SetDynamicConstantBufferView(PathTracing::GlobalRootSignatureBindings::GlobalRayTracingConstants, sizeof(RTCB), &RTCB);


		UINT frameResourceIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();
		// Global SRVs
		{
			DVector<D3D12_CPU_DESCRIPTOR_HANDLE> srvHandles;
			srvHandles.reserve(3);
			srvHandles.push_back(RTScene->GetTopLevelAS().GetSRV());
			if (renderContext.IrradianceIBL != nullptr && renderContext.RadianceIBL != nullptr)
			{
				srvHandles.push_back(renderContext.RadianceIBL->GetSRV());
				srvHandles.push_back(renderContext.IrradianceIBL->GetSRV());
			}
			else
			{
				srvHandles.push_back(BlackCubeTextureRes->GetTextureData()->GetSRV());
				srvHandles.push_back(BlackCubeTextureRes->GetTextureData()->GetSRV());
			}
			UINT srvSrcCount[] = { 1u, 1u, 1u };
			UINT srvDestCount = 3u;

			D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &PathTracingCommonSRVs[frameResourceIndex], &srvDestCount, (UINT)srvHandles.size(), srvHandles.data(), srvSrcCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			context.SetDescriptorTable(PathTracing::GlobalRootSignatureBindings::GlobalSRVTable, PathTracingCommonSRVs[frameResourceIndex]);
		}
		
		// Global Light Data
		{
			D3D12_CPU_DESCRIPTOR_HANDLE srvHandles[] =
			{
				LightContext->GetLightsStatusBufferDescriptor(),
				LightContext->GetLightsDataBufferDescriptor()
			};
			
			UINT srvSrcCount[] = { 1u, 1u };
			UINT srvDestCount = 2u;
			D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &PathTracingLightDataSRVs[frameResourceIndex], &srvDestCount, 2u, srvHandles, srvSrcCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			context.SetDescriptorTable(PathTracing::GlobalRootSignatureBindings::GlobalLightData, PathTracingLightDataSRVs[frameResourceIndex]);
		}
	}

	void CreateLocalBindings(RayTracingCommandContext& context, SceneRenderContext& renderContext, ShaderTable* shaderTable)
	{
		
		// Ray Generation
		auto rayGenShader = MainRenderPipeline->GetStateObject()->GetRayGenerationShaders()[0];
		UINT rayGenUavSrcCount[] = { 1u, 1u, 1u, 1u };
		UINT rayGenUavDestCount = 4;
		D3D12_CPU_DESCRIPTOR_HANDLE rayGenUavSrcs[] = { renderContext.ColorBuffer.GetUAV(), renderContext.WorldPos.GetUAV(), renderContext.NormalDepth.GetUAV(), renderContext.LinearDepth->GetUAV() };
		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &RayGenUAVs[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()], &rayGenUavDestCount, 4, rayGenUavSrcs, rayGenUavSrcCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		shaderTable->SetRayGenerationShaderParameters(0, rayGenShader->GetLocalRootSignature()->GetRootParameterOffset(0), RayGenUAVs[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()].GetGpuPtr());


		// Hit groups
		auto hitGroup = MainRenderPipeline->GetStateObject()->GetHitGroups()[0];
		auto CHSRS = hitGroup.ClosestHitShader->GetLocalRootSignature();
		// For each instance
		for (UINT instanceIndex = 0u; instanceIndex < RTScene->GetNumberOfBottomLevelASInstances(); instanceIndex++)
		{
			auto const* blas = RTScene->GetBLASByInstanceIndex(instanceIndex);
			auto const& inst = RTScene->GetBottomLevelASInstance(instanceIndex);

			auto const& instanceData = RTScene->GetInstanceData(instanceIndex);
			auto const& materials = instanceData.GeometriesMaterial;
			auto meshConstantsAddress = instanceData.MeshConstantsAddress;

			// For each geom in instance
			for (UINT geomIndex = 0u; geomIndex < blas->GetNumGeometries(); geomIndex++)
			{
				auto const& meshVertViews = blas->GetGeometryMeshViewsByIndex(geomIndex);
				
				// For now, no settings for shadow
				for (UINT geomSlot = 0; geomSlot < (UINT)RayTypes::Shadow; geomSlot++)
				{
					UINT hitGroupIndex = (RTScene->GetTotalNumberOfPriorGeometrySegmets(instanceIndex) + geomIndex) * (UINT)RayTypes::Count + geomSlot;

					shaderTable->SetHitGroupIdentifier(hitGroupIndex, hitGroup.Identifier);
					

					// Settting mesh vertices data		RootIndex 0
					shaderTable->SetHitGroupSystemParameters(hitGroupIndex, { meshVertViews.IndexVertexBufferSRV });

					// Setting mesh constants			RootIndex 1
					shaderTable->SetHitGroupParameters(hitGroupIndex, CHSRS->GetRootParameterOffset(1), meshConstantsAddress);

					// Setting material constans		RootIndex 2
					shaderTable->SetHitGroupParameters(hitGroupIndex, CHSRS->GetRootParameterOffset(2), materials[geomIndex].MaterialConstants);

					// Setting material textures		RootIndex 3
					shaderTable->SetHitGroupParameters(hitGroupIndex, CHSRS->GetRootParameterOffset(3), materials[geomIndex].TextureTableHandle);

					// Setting material samplers		RootIndex 4
					shaderTable->SetHitGroupParameters(hitGroupIndex, CHSRS->GetRootParameterOffset(4), materials[geomIndex].SamplerTableHandle);

				}

			}
		}

	}

	void CreateFrameBindings(RayTracingCommandContext& context, SceneRenderContext& renderContext)
	{
		D_PROFILING::ScopedTimer _prof(L"Create Frame Bindings");
		auto const* stateObj = MainRenderPipeline->GetStateObject();
		auto shaderTable = RTScene->FindOrCreateShaderTable(stateObj);

		CreateGlobalBindings(context, renderContext, shaderTable);

		CreateLocalBindings(context, renderContext, shaderTable);

		shaderTable->CopyToGpu(context);
	}

	void Render(std::wstring const& jobId, SceneRenderContext& renderContext, std::function<void()> postAntiAliasing)
	{
		RayTracingCommandContext& context = static_cast<RayTracingCommandContext&>(renderContext.CommandContext);
		auto& renderTarget = renderContext.ColorBuffer;
		auto const& camera = renderContext.Camera;

		D_PROFILING::ScopedTimer _prof(L"Ray Tracing Render", context);

		// Update Lights
		{

			D_PROFILING::ScopedTimer _prof(L"Update Shadowed Light Context", context);

			LightContext->Update(camera);
			LightContext->UpdateBuffers(context, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

		RTScene->Build(context, nullptr, D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex(), false);

		CreateFrameBindings(context, renderContext);

		context.TransitionResource(renderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		auto shaderTable = RTScene->FindExistingShaderTable(MainRenderPipeline->GetStateObject());
		D_ASSERT(shaderTable);

		D3D12_DISPATCH_RAYS_DESC rayTracingDesc = shaderTable->GetDispatchRaysDesc(0u, true);

		rayTracingDesc.Width = renderTarget.GetWidth();
		rayTracingDesc.Height = renderTarget.GetHeight();
		rayTracingDesc.Depth = 1u;

		context.DispatchRays(&rayTracingDesc);


		//if (postAntiAliasing)
			//postAntiAliasing();
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_INT_SLIDER("Max Number of Buttom Level AS", "RayTracing.MaxNumBottomLevelAS", MaxNumBottomLevelAS, 10, 1000000);

		D_H_OPTION_DRAW_END();
	}
#endif

	DescriptorHandle AllocateTextureDescriptor(UINT count)
	{
		return TextureHeap.Alloc(count);
	}

	DescriptorHandle AllocateSamplerDescriptor(UINT count)
	{
		return SamplerHeap.Alloc(count);
	}

}