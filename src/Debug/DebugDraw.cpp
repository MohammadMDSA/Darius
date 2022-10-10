#include "pch.hpp"
#include "DebugDraw.hpp"

#include <Renderer/RenderDeviceManager.hpp>

#include <mutex>

#define MAX_DEBUG_DRAWS 1024

using namespace D_RESOURCE;

namespace Darius::Debug
{

#ifdef _DEBUG

	D_CORE::Ref<D_RESOURCE::MeshResource>	CubeMeshResource;

	// Gpu buffers
	D_GRAPHICS_BUFFERS::UploadBuffer		MeshConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
	ByteAddressBuffer						MeshConstantsGPU;

	std::mutex								AdditionMutex;

	D_CONTAINERS::DVector<D_RENDERER_FRAME_RESOUCE::RenderItem> DrawPending;


#endif // _DEBUG

	void DebugDraw::Initialize()
	{
#ifdef _DEBUG
		CubeMeshResource = GetResource<D_RESOURCE::MeshResource>(GetDefaultResource(D_RESOURCE::DefaultResource::BoxMesh));

		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			MeshConstantsCPU[i].Create(L"Debug Mesh Constant Upload Buffer", sizeof(D_RENDERER_FRAME_RESOUCE::MeshConstants) * MAX_DEBUG_DRAWS);
		}
		MeshConstantsGPU.Create(L"Debug Mesh Constant GPU Buffer", MAX_DEBUG_DRAWS, sizeof(D_RENDERER_FRAME_RESOUCE::MeshConstants));

#endif // _DEBUG

	}

	void DebugDraw::Shutdown()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			MeshConstantsCPU[i].Destroy();
		}
		MeshConstantsGPU.Destroy();
	}

	void DebugDraw::DrawCube(D_MATH::Vector3 position, D_MATH::Quaternion rotation, D_MATH::Vector3 scale, D_MATH::Color color)
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);

		if (DrawPending.size() >= MAX_DEBUG_DRAWS)
			return;

		auto index = DrawPending.size();

		// Upload transform
		UploadTransform(D_MATH::Transform(position, rotation, scale), index);

		D_RENDERER_FRAME_RESOUCE::RenderItem ri;
		PopulateRenderItemFromMesh(ri, CubeMeshResource->GetMeshData());
		ri.MeshCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER_FRAME_RESOUCE::MeshConstants) * index;
		ri.Color = color;
		ri.PsoType = D_RENDERER::ColorWireframeTwoSidedPso;
		ri.PsoFlags = D_RENDERER_FRAME_RESOUCE::RenderItem::ColorOnly | D_RENDERER_FRAME_RESOUCE::RenderItem::Wireframe;
		D_LOG_DEBUG(index);
		DrawPending.push_back(ri);
	}

	void DebugDraw::PopulateRenderItemFromMesh(D_RENDERER_FRAME_RESOUCE::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh)
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
		cb.mWorldIT = InverseTranspose(Matrix3(world));

		currentUploadBuff.Unmap();

	}

	void DebugDraw::FinalizeUpload()
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		if (DrawPending.size() <= 0)
			return;
		D_LOG_DEBUG("_________");

		auto& currentUploadBuff = MeshConstantsCPU[D_RENDERER_DEVICE::GetCurrentResourceIndex()];

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Debug Mesh CBV Upload");
		context.TransitionResource(MeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(MeshConstantsGPU.GetResource(), 0, currentUploadBuff.GetResource(), 0, sizeof(D_RENDERER_FRAME_RESOUCE::MeshConstants) * DrawPending.size());
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

	void DebugDraw::Clear()
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		DrawPending.clear();
	}

}
