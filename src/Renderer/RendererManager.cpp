#include "pch.hpp"
#include "RendererManager.hpp"

#include "Components/BillboardRendererComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/LightComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
#include "Components/SkeletalMeshRendererComponent.hpp"
#include "Components/TerrainRendererComponent.hpp"
#include "Geometry/GeometryGenerator.hpp"
#include "RayTracing/Renderer.hpp"
#include "Rasterization/Renderer.hpp"
#include "Resources/BatchResource.hpp"
#include "Resources/MaterialResource.hpp"
#include "Resources/ShaderMaterialResource.hpp"
#include "Resources/SkeletalMeshResource.hpp"
#include "Resources/ShaderResource.hpp"
#include "Resources/StaticMeshResource.hpp"
#include "Resources/TerrainResource.hpp"
#include "Resources/TextureResource.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Containers/Set.hpp>
#include <Core/Uuid.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Math/VectorMath.hpp>
#include <Utils/Assert.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_GRAPHICS;
using namespace D_GRAPHICS_MEMORY;
using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;

namespace Darius::Renderer
{

	bool												_initialized = false;
	RendererType										ActiveRendererType;

	// Settings
	bool												HardwareRayTracing;


	// Heaps
	DescriptorHeap										TextureHeap;
	DescriptorHeap										SamplerHeap;


	DUnorderedMap<DefaultResource, ResourceHandle>		DefaultResourceMap;

	void												LoadDefaultResources();
	D_MATH_BOUNDS::DynamicBVH<D_ECS::UntypedCompRef>	SceneBVH;

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		D_H_OPTIONS_LOAD_BASIC_DEFAULT("Renderer.HardwareRayTracing", HardwareRayTracing, false);

		// Initializing Resources
		TextureResource::Register();
		StaticMeshResource::Register();
		SkeletalMeshResource::Register();
		MaterialResource::Register();
		BatchResource::Register();
		TerrainResource::Register();
		ShaderResource::Register();
		ShaderMaterialResource::Register();

		LoadDefaultResources();

		ActiveRendererType = HardwareRayTracing ? RendererType::RayTracing : RendererType::Rasterization;

		// Creating heaps
		TextureHeap.Create(L"Renderer SRV, UAV, CBV  Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
		SamplerHeap.Create(L"Renderer Sampler  Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);


		if (ActiveRendererType == RendererType::Rasterization)
			D_RENDERER_RAST::Initialize(settings);
		else
			D_RENDERER_RT::Initialize(settings);

		// Registering components
		BillboardRendererComponent::StaticConstructor();
		CameraComponent::StaticConstructor();
		LightComponent::StaticConstructor();
		MeshRendererComponent::StaticConstructor();
		SkeletalMeshRendererComponent::StaticConstructor();
		TerrainRendererComponent::StaticConstructor();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		if (ActiveRendererType == RendererType::Rasterization)
			D_RENDERER_RAST::Shutdown();
		else
			D_RENDERER_RT::Shutdown();

		TextureHeap.Destroy();
		SamplerHeap.Destroy();
	}

	void Update()
	{
		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Update Renderer");

		switch (ActiveRendererType)
		{
		case Darius::Renderer::RendererType::Rasterization:
			D_RENDERER_RAST::Update(context);
			break;
		case Darius::Renderer::RendererType::RayTracing:
			D_RENDERER_RT::Update(context);
			break;
		default:
			D_ASSERT_M(true, "Update method not been implemented for this renderer type");
		}

		{
			D_PROFILING::ScopedTimer _prof(L"Optimizing Scene BVH", context);
			SceneBVH.OptimizeIncremental(1);
		}

		context.Finish();
	}

	void Render(std::wstring const& jobId, SceneRenderContext& renderContext, std::function<void()> postAntiAliasing)
	{
		switch (ActiveRendererType)
		{
		case Darius::Renderer::RendererType::Rasterization:
			D_RENDERER_RAST::Render(jobId, renderContext, postAntiAliasing);
			break;
		case Darius::Renderer::RendererType::RayTracing:
			D_RENDERER_RT::Render(jobId, renderContext, postAntiAliasing);
			break;
		default:
			D_ASSERT_M(true, "Render method not been implemented for this renderer type");
		}
	}

	D_RESOURCE::ResourceHandle GetDefaultGraphicsResource(DefaultResource type)
	{
		return DefaultResourceMap[type];
	}

	void LoadDefaultResources()
	{
		// Creating default meshes
		{
			auto box = D_RENDERER_GEOMETRY_GENERATOR::CreateBox(1.f, 1.f, 1.f, 0);
			auto cylinder = D_RENDERER_GEOMETRY_GENERATOR::CreateCylinder(0.5f, 0.5f, 1, 40, 20);
			auto geosphere = D_RENDERER_GEOMETRY_GENERATOR::CreateGeosphere(0.5f, 40);
			auto gridp2 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(100.f, 100.f, 2, 2);
			auto gridp4 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(100.f, 100.f, 4, 4);
			auto gridp8 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(100.f, 100.f, 8, 8);
			auto gridp16 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(100.f, 100.f, 16, 16);
			auto grid100 = D_RENDERER_GEOMETRY_GENERATOR::CreateGrid(100.f, 100.f, 1000, 1000);
			auto quad = D_RENDERER_GEOMETRY_GENERATOR::CreateQuad(0.f, 0.f, 1.f, 1.f, 0.f);
			auto sphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 40, 40);
			auto lowSphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 10, 6);
			auto line = D_RENDERER_GEOMETRY_GENERATOR::CreateLine(0.f, 0.f, 0.f, 0.f, 0.f, -1.f);

			auto resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Box Mesh"), L"Box Mesh", L"Box Mesh", true);
			MultiPartMeshData<StaticMeshResource::VertexType> meshData;
			meshData.MeshData = box;
			auto res = D_RESOURCE::GetRawResourceSync(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::BoxMesh, { StaticMeshResource::GetResourceType(), res->GetId()} });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Cylinder Mesh"), L"Cylinder Mesh", L"Cylinder Mesh", true);
			meshData.MeshData = cylinder;
			res = D_RESOURCE::GetRawResourceSync(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::CylinderMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Geosphere Mesh"), L"Geosphere Mesh", L"Geosphere Mesh", true);
			meshData.MeshData = geosphere;
			res = D_RESOURCE::GetRawResourceSync(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::GeosphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			{
				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 2x2 Mesh"), L"Grid 2x2 Mesh", L"Grid 2x2 Mesh", true);
				meshData.MeshData = gridp2;
				res = D_RESOURCE::GetRawResourceSync(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch2x2Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 4x4 Mesh"), L"Grid 4x4 Mesh", L"Grid 4x4 Mesh", true);
				meshData.MeshData = gridp4;
				res = D_RESOURCE::GetRawResourceSync(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch4x4Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 8x8 Mesh"), L"Grid 8x8 Mesh", L"Grid 8x8 Mesh", true);
				meshData.MeshData = gridp8;
				res = D_RESOURCE::GetRawResourceSync(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch8x8Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 16x16 Mesh"), L"Grid 16x16 Mesh", L"Grid 16x16 Mesh", true);
				meshData.MeshData = gridp16;
				res = D_RESOURCE::GetRawResourceSync(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch16x16Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 100x100 Mesh"), L"Grid 100x100 Mesh", L"Grid 100x100 Mesh", true);
				meshData.MeshData = grid100;
				res = D_RESOURCE::GetRawResourceSync(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::Grid100x100Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });
			}

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Quad Mesh"), L"Quad Mesh", L"Quad Mesh", true);
			meshData.MeshData = quad;
			res = D_RESOURCE::GetRawResourceSync(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::QuadMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Sphere Mesh"), L"Sphere Mesh", L"Sphere Mesh", true);
			meshData.MeshData = sphere;
			res = D_RESOURCE::GetRawResourceSync(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::SphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Low Poly Sphere Mesh"), L"Low Poly Sphere Mesh", L"Low Poly Sphere Mesh", true);
			meshData.MeshData = lowSphere;
			res = D_RESOURCE::GetRawResourceSync(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::LowPolySphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<BatchResource>(GenerateUuidFor("Line Mesh"), L"Line Mesh", L"Line Mesh", true);
			meshData.MeshData = line;
			res = D_RESOURCE::GetRawResourceSync(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((BatchResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::LineMesh, { BatchResource::GetResourceType(), res->GetId() } });
		}

		// Create default textures
		{
#define CreateDefaultTexture2D(name, color) \
{ \
	auto defaultTextureHandle = D_RESOURCE::GetManager()->CreateResource<TextureResource>(GenerateUuidFor("Default Texture2D " #name), L"Default Texture2D " #name, L"Default Texture2D " #name, true); \
	auto textureRes = (TextureResource*)D_RESOURCE::GetRawResourceSync(defaultTextureHandle); \
	textureRes->CreateRaw(color, DXGI_FORMAT_R8G8B8A8_UNORM, 4, 1, 1); \
	auto rRes = dynamic_cast<Resource*>(textureRes); \
	rRes->MakeGpuClean(); \
	rRes->MakeDiskClean(); \
	DefaultResourceMap.insert({ DefaultResource::Texture2D##name, { TextureResource::GetResourceType(), textureRes->GetId() } }); \
}

#define CreateDefaultTextureCubeMap(name, color) \
{ \
	auto defaultTextureHandle = D_RESOURCE::GetManager()->CreateResource<TextureResource>(GenerateUuidFor("Default TextureCubeMap" #name), L"Default Texture2D " #name, L"Default Texture2D " #name, true); \
	auto textureRes = (TextureResource*)D_RESOURCE::GetRawResourceSync(defaultTextureHandle); \
	textureRes->CreateCubeMap(color, DXGI_FORMAT_R8G8B8A8_UNORM, 4, 1, 1); \
	auto rRes = dynamic_cast<Resource*>(textureRes); \
	rRes->MakeGpuClean(); \
	rRes->MakeDiskClean(); \
	DefaultResourceMap.insert({ DefaultResource::TextureCubeMap##name, { TextureResource::GetResourceType(), textureRes->GetId() } }); \
}
			CreateDefaultTexture2D(Magenta, 0xFFFF00FF);
			CreateDefaultTexture2D(BlackOpaque, 0xFF000000);
			CreateDefaultTexture2D(BlackTransparent, 0x00000000);
			CreateDefaultTexture2D(WhiteOpaque, 0xFFFFFFFF);
			CreateDefaultTexture2D(WhiteTransparent, 0x00FFFFFF);
			CreateDefaultTexture2D(NormalMap, 0x00FF0000);

			uint32_t blackCubeTexels[6] = {};
			CreateDefaultTextureCubeMap(Black, blackCubeTexels);
		}

		// Creating default materials
		{
			auto defaultMaterialHandle = D_RESOURCE::GetManager()->CreateResource<MaterialResource>(GenerateUuidFor("Default Material"), L"Default Material", L"Default Material", true);
			auto materialRes = (MaterialResource*)D_RESOURCE::GetRawResourceSync(defaultMaterialHandle);
			auto mat = materialRes->ModifyMaterialData();
			mat->DifuseAlbedo = DirectX::XMFLOAT4(Vector4(kOne));
			mat->FresnelR0 = DirectX::XMFLOAT3(Vector3(kOne) * 0.56f);
			mat->Metallic = 0.f;
			mat->Roughness = 0.f;
			auto rRes = dynamic_cast<Resource*>(materialRes);
			DefaultResourceMap.insert({ DefaultResource::Material, { MaterialResource::GetResourceType(), materialRes->GetId() } });
		}

	}

	RendererType				GetActiveRendererType()
	{
		return ActiveRendererType;
	}

	void SetIBLTextures(D_RENDERER::TextureResource* diffuseIBL, D_RENDERER::TextureResource* specularIBL)
	{
		switch (ActiveRendererType)
		{
		case Darius::Renderer::RendererType::Rasterization:
			D_RENDERER_RAST::SetIBLTextures(diffuseIBL, specularIBL);
			break;
		case Darius::Renderer::RendererType::RayTracing:
			break;
		default:
			D_ASSERT_M(true, "Set IBL Texture method not been implemented for this renderer type");
		}
	}

	void SetIBLBias(float LODBias)
	{
		switch (ActiveRendererType)
		{
		case Darius::Renderer::RendererType::Rasterization:
			D_RENDERER_RAST::SetIBLBias(LODBias);
			break;
		case Darius::Renderer::RendererType::RayTracing:
			break;
		default:
			D_ASSERT_M(true, "Set IBL Bias method not been implemented for this renderer type");
		}
	}

	void SetForceWireframe(bool val)
	{
		switch (ActiveRendererType)
		{
		case Darius::Renderer::RendererType::Rasterization:
			D_RENDERER_RAST::SetForceWireframe(val);
			break;
		case Darius::Renderer::RendererType::RayTracing:
			break;
		default:
			D_ASSERT_M(true, "Set IBL Bias method not been implemented for this renderer type");
		}
	}

	DescriptorHandle AllocateTextureDescriptor(UINT count)
	{
		return TextureHeap.Alloc(count);
	}

	DescriptorHandle AllocateSamplerDescriptor(UINT count)
	{
		return SamplerHeap.Alloc(count);
	}

	uint32_t GetTextureDescriptorHeapDescriptorSize()
	{
		return TextureHeap.GetDescriptorSize();
	}

	uint32_t GetSamplerDescriptorHeapDescriptorSize()
	{
		return SamplerHeap.GetDescriptorSize();
	}

	void SetCbvSrvUavDescriptorHeap(D_GRAPHICS::CommandContext& context)
	{
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
	}

	void SetSamplerDescriptorHeap(D_GRAPHICS::CommandContext& context)
	{
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SamplerHeap.GetHeapPointer());
	}

	DynamicBVH<D_ECS::UntypedCompRef>::ID RegisterComponent(D_ECS::UntypedCompRef const& compRef)
	{
		D_ASSERT(compRef.IsValid());
		auto comp = reinterpret_cast<RendererComponent*>(compRef.Get());
		return SceneBVH.Insert(comp->GetAabb(), compRef);
	}

	bool UpdateComponentBounds(DynamicBVH<D_ECS::UntypedCompRef>::ID const& id, Aabb const& aabb)
	{
		return SceneBVH.Update(id, aabb);
	}

	void UnregisterComponent(DynamicBVH<D_ECS::UntypedCompRef>::ID const& id)
	{
		SceneBVH.Remove(id);
	}

	D_MATH_BOUNDS::DynamicBVH<D_ECS::UntypedCompRef> const& GetSceneBvh()
	{
		return SceneBVH;
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_CHECKBOX("Hardware Ray Tracing", "Renderer.HardwareRayTracing", HardwareRayTracing);
		if((HardwareRayTracing && ActiveRendererType != RendererType::RayTracing) || (!HardwareRayTracing && ActiveRendererType == RendererType::RayTracing))
		{
			ImGui::SameLine();
			ImGui::TextColored({1.f, 1.f, 0.f, 1.f}, "You have to restart the engine for this option to take effect!");
		}

		ImGui::Separator();

		if(ImGui::CollapsingHeader("Rasterization"))
		{
			ImGui::Indent();
			ImGui::BeginGroup();
			settingsChanged |= D_RENDERER_RAST::OptionsDrawer(options);
			ImGui::EndGroup();
		}

		D_H_OPTION_DRAW_END()
}
#endif

}
