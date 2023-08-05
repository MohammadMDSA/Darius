#include "Renderer/pch.hpp"
#include "Renderer.hpp"

#include "Pipelines/SimpleRayTracingRenderer.hpp"
#include "RayTracingScene.hpp"
#include "Renderer/Components/MeshRendererComponent.hpp"

#include <Core/Uuid.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <ResourceManager/ResourceManager.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

constexpr D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS StaticBLASBuildFlags =
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;

using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_MEMORY;
using namespace D_RENDERER_RT_UTILS;

namespace Darius::Renderer::RayTracing
{

	// Settings
	UINT													MaxNumBottomLevelAS;

	// Scene
	std::unique_ptr<RayTracingScene>						RTScene;

	// Pipelines
	std::unique_ptr<Pipeline::SimpleRayTracingPipeline>		SimpleRayTracingRenderer;

	// Heaps
	DescriptorHeap											TextureHeap;
	DescriptorHeap											SamplerHeap;

	// Internal
	bool													_initialized;
	D_GRAPHICS_BUFFERS::ByteAddressBuffer					ColorCBV;
	DescriptorHandle										SceneColorTextureDescriptor;

	ALIGN_DECL_256 struct MeshConstants
	{
		DirectX::XMFLOAT3		colors[3];
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

		SimpleRayTracingRenderer = std::make_unique<Pipeline::SimpleRayTracingPipeline>();
		SimpleRayTracingRenderer->Initialize(settings);

		MeshConstants color = { { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} } };
		ColorCBV.Create(L"Simple Ray Tracing Color CBV", 1, sizeof(MeshConstants), &color);

		SceneColorTextureDescriptor = TextureHeap.Alloc(1);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		SimpleRayTracingRenderer->Shutdown();

		RTScene.reset();
		SimpleRayTracingRenderer.reset();

		TextureHeap.Destroy();
		SamplerHeap.Destroy();
	}

	void Update()
	{

		RTScene->Reset();

		// Updating BLASes
		{
			auto createStaticMeshBlas = [scene = RTScene.get()](D_RENDERER_GEOMETRY::Mesh const& mesh, D_CORE::Uuid const& uuid)
			{
				// TODO: Add translucent objects
				BottomLevelAccelerationStructureGeometry BLASGeom = { mesh, uuid, D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE };
				RTScene->AddBottomLevelAS(StaticBLASBuildFlags, BLASGeom, false, false);
			};

			D_WORLD::IterateComponents<MeshRendererComponent>([scene = RTScene.get(), createBlasFunc = createStaticMeshBlas](D_RENDERER::MeshRendererComponent& comp)
				{
					auto const* meshRes = comp.GetMesh();
					auto uuid = meshRes->GetUuid();
					auto const* blas = scene->GetBottomLevelAS(uuid);

					// Create blas if it's not there
					if (!blas)
					{
						createBlasFunc(*meshRes->GetMeshData(), uuid);
					}

					scene->AddBottomLevelASInstance(uuid, comp.GetTransform()->GetWorld());
				});

		}
	}

	void CreateFrameBindings(RayTracingCommandContext& context, ColorBuffer& sceneColor)
	{
		D_PROFILING::ScopedTimer _prof(L"Create Frame Bindings");
		auto const* stateObj = SimpleRayTracingRenderer->GetStateObject();
		auto shaderTable = RTScene->FindOrCreateShaderTable(stateObj);
		
		auto hitGroup = stateObj->GetHitGroups()[0];

		for (UINT i = 0; i < RTScene->GetTotalNumberOfGeometrySegments() * (UINT)RayTypes::Count; i++)
		{
			shaderTable->SetHitGroupIdentifier(i, hitGroup.Identifier);
			shaderTable->SetHitGroupParameters(i, 0, ColorCBV.GetGpuVirtualAddress());
		}

		auto rayGenShader = SimpleRayTracingRenderer->GetStateObject()->GetRayGenerationShaders()[0];
		
		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1u, SceneColorTextureDescriptor, sceneColor.GetUAV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		shaderTable->SetRayGenerationShaderParameters(0, rayGenShader->GetLocalRootSignature()->GetRootParameterOffset(1), SceneColorTextureDescriptor.GetGpuPtr());

		shaderTable->CopyToGpu(context);	
	}

	void Render(std::wstring const& jobId, SceneRenderContext& renderContext, std::function<void()> postAntiAliasing)
	{
		RayTracingCommandContext& context = static_cast<RayTracingCommandContext&>(renderContext.CommandContext);
		auto& renderTarget = renderContext.ColorBuffer;
		auto const& camera = renderContext.Camera;

		D_PROFILING::ScopedTimer _prof(L"Simple Ray Tracing Render", context);

		RTScene->Build(context, nullptr, D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex(), false);

		CreateFrameBindings(context, renderTarget);

		context.TransitionResource(renderTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);

		auto shaderTable = RTScene->FindExistingShaderTable(SimpleRayTracingRenderer->GetStateObject());
		D_ASSERT(shaderTable);

		D3D12_DISPATCH_RAYS_DESC rayTracingDesc = shaderTable->GetDispatchRaysDesc(0u, true);

		rayTracingDesc.Width = renderTarget.GetWidth();
		rayTracingDesc.Height = renderTarget.GetHeight();
		rayTracingDesc.Depth = 1u;

		context.GetComputeContext().SetRootSignature(*SimpleRayTracingRenderer->GetStateObject()->GetGlobalRootSignature());

		context.SetDynamicConstantBufferView(0, sizeof(renderContext.Globals), &renderContext.Globals);

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SamplerHeap.GetHeapPointer());
		context.SetPipelineState(*SimpleRayTracingRenderer->GetStateObject());

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