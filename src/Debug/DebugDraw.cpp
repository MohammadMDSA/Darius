#include "pch.hpp"
#include "DebugDraw.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Renderer/RenderDeviceManager.hpp>

#include <mutex>

#define MAX_DEBUG_DRAWS 1024

#define RENDERSETUP(meshResource) \
D_RENDERER_FRAME_RESOURCE::RenderItem ri; \
PopulateRenderItemFromMesh(ri, meshResource->GetMeshData()); \
ri.MeshCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants) * index; \
ri.Color = color; \
ri.PsoFlags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::Wireframe; \
ri.PsoType = D_RENDERER::GetPso(ri.PsoFlags); \
DrawPending.push_back(ri); \
if(duration > 0) \
{ \
	DrawsWithDuration.insert({ D_TIME::GetTotalTime() + duration, { ri, trans } }); \
}

using namespace D_RESOURCE;

namespace Darius::Debug
{

#ifdef _DEBUG

	// Gpu buffers
	D_GRAPHICS_BUFFERS::UploadBuffer		MeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
	ByteAddressBuffer						MeshConstantsGPU;

	std::mutex								AdditionMutex;

	D_CONTAINERS::DVector<D_RENDERER_FRAME_RESOURCE::RenderItem> DrawPending;

	D_CONTAINERS::DUnorderedMap<double, std::pair<D_RENDERER_FRAME_RESOURCE::RenderItem, D_MATH::Transform>> DrawsWithDuration;

	D_CORE::Ref<D_GRAPHICS::StaticMeshResource>	DebugDraw::CubeMeshResource;
	D_CORE::Ref<D_GRAPHICS::StaticMeshResource>	DebugDraw::SphereMeshResource;

#endif // _DEBUG

	void DebugDraw::Initialize(D_SERIALIZATION::Json const& settings)
	{
#ifdef _DEBUG
		CubeMeshResource = GetResource<D_GRAPHICS::StaticMeshResource>(D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::BoxMesh));
		SphereMeshResource = GetResource<D_GRAPHICS::StaticMeshResource>(D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::LowPolySphereMesh));

		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			MeshConstantsCPU[i].Create(L"Debug Mesh Constant Upload Buffer", sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants) * MAX_DEBUG_DRAWS);
		}
		MeshConstantsGPU.Create(L"Debug Mesh Constant GPU Buffer", MAX_DEBUG_DRAWS, sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants));

#endif // _DEBUG

	}

	void DebugDraw::Shutdown()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			MeshConstantsCPU[i].Destroy();
		}
		MeshConstantsGPU.Destroy();
	}

#ifdef _D_EDITOR
	bool DebugDraw::OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	void DebugDraw::DrawCube(D_MATH::Vector3 position, D_MATH::Quaternion rotation, D_MATH::Vector3 scale, double duration, D_MATH::Color color)
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);

		if (DrawPending.size() >= MAX_DEBUG_DRAWS)
			return;

		auto index = DrawPending.size();

		// Upload transform
		auto trans = D_MATH::Transform(position, rotation, scale);
		UploadTransform(trans, index);

		RENDERSETUP(CubeMeshResource);
	}

	void DebugDraw::DrawSphere(D_MATH::Vector3 position, float radius, double duration, D_MATH::Color color)
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		if (DrawPending.size() >= MAX_DEBUG_DRAWS)
			return;

		auto index = DrawPending.size();

		auto trans = D_MATH::Transform(position, Quaternion(kIdentity), { radius, radius, radius });
		UploadTransform(trans, index);

		RENDERSETUP(SphereMeshResource);
	}

	void DebugDraw::PopulateRenderItemFromMesh(D_RENDERER_FRAME_RESOURCE::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh)
	{
		renderItem.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		renderItem.IndexCount = mesh->mNumTotalIndices;
		renderItem.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		renderItem.Mesh = mesh;
		renderItem.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	void DebugDraw::UploadTransform(D_MATH::Transform const& trans, UINT index)
	{
		// Updating mesh constants
		// Mapping upload buffer
		auto& currentUploadBuff = MeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		MeshConstants& cb = ((MeshConstants*)currentUploadBuff.Map())[index];

		auto world = trans.GetWorld();
		cb.mWorld = Matrix4(world);
		//cb.mWorldIT = InverseTranspose(Matrix3(world));

		currentUploadBuff.Unmap();

	}

	void DebugDraw::FinalizeUpload()
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		if (DrawPending.size() <= 0)
			return;

		auto& currentUploadBuff = MeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Debug Mesh CBV Upload");
		context.TransitionResource(MeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(MeshConstantsGPU.GetResource(), 0, currentUploadBuff.GetResource(), 0, sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants) * DrawPending.size());
		context.TransitionResource(MeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();
	}

	void DebugDraw::GetRenderItems(D_RENDERER::MeshSorter& sorter)
	{
		//D_LOG_DEBUG(DrawPending.size());
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		for (auto const& item : DrawPending)
		{
			sorter.AddMesh(item, -1);
		}
	}

	void DebugDraw::Clear(bool clearCache)
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		DrawPending.clear();

		if (clearCache)
			DrawsWithDuration.clear();
		
		auto now = D_TIME::GetTotalTime();

		int index = 0;

		for (auto it = DrawsWithDuration.begin(); it != DrawsWithDuration.end();)
		{
			if (it->first < now)
				DrawsWithDuration.erase(it++);
			else
			{
				UploadTransform(it->second.second, index);
				auto& ri = it->second.first;
				ri.MeshCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants) * index;
				DrawPending.push_back(ri);
				index++;
				it++;
			}
		}
	}

}
