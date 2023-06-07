#include "pch.hpp"
#include "DebugDraw.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Renderer/GraphicsDeviceManager.hpp>

#include <mutex>

#define MAX_DEBUG_DRAWS 1024


#define RENDERSETUP_LINE(meshResource) \
D_RENDERER_FRAME_RESOURCE::RenderItem ri; \
PopulateRenderItemFromMesh(ri, meshResource->GetMeshData()); \
ri.MeshVsCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants) * index; \
ri.Color = color; \
ri.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST; \
ri.PsoFlags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::LineOnly | RenderItem::AlphaBlend; \
ri.PsoType = D_RENDERER::GetPso({ ri.PsoFlags }); \
ri.DepthPsoIndex = D_RENDERER::GetPso({ (UINT16)(ri.PsoFlags | RenderItem::DepthOnly) }); \
DrawPending.push_back(ri); \
if(duration > 0) \
{ \
	DrawsWithDuration.insert({ D_TIME::GetTotalTime() + duration, { ri, trans } }); \
}

#define RENDERSETUP(meshResource) \
D_RENDERER_FRAME_RESOURCE::RenderItem ri; \
PopulateRenderItemFromMesh(ri, meshResource->GetMeshData()); \
ri.MeshVsCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants) * index; \
ri.Color = color; \
ri.PsoFlags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::Wireframe; \
ri.PsoType = D_RENDERER::GetPso({ ri.PsoFlags }); \
ri.DepthPsoIndex = D_RENDERER::GetPso({ (UINT16)(ri.PsoFlags | RenderItem::DepthOnly) }); \
DrawPending.push_back(ri); \
if(duration > 0) \
{ \
	DrawsWithDuration.insert({ D_TIME::GetTotalTime() + duration, { ri, trans } }); \
}

using namespace D_MATH;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace D_RESOURCE;

namespace Darius::Debug
{

#ifdef _DEBUG

	// Gpu buffers
	D_GRAPHICS_BUFFERS::UploadBuffer		MeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
	D_GRAPHICS_BUFFERS::ByteAddressBuffer	MeshConstantsGPU;

	std::mutex								AdditionMutex;

	D_CONTAINERS::DVector<D_RENDERER_FRAME_RESOURCE::RenderItem> DrawPending;

	D_CONTAINERS::DUnorderedMap<double, std::pair<D_RENDERER_FRAME_RESOURCE::RenderItem, D_MATH::Transform>> DrawsWithDuration;

	D_RESOURCE::ResourceRef<D_GRAPHICS::StaticMeshResource>	DebugDraw::CubeMeshResource({ L"Debug Drawer" });
	D_RESOURCE::ResourceRef<D_GRAPHICS::StaticMeshResource>	DebugDraw::SphereMeshResource({ L"Debug Drawer" });
	D_RESOURCE::ResourceRef<D_GRAPHICS::BatchResource>		DebugDraw::LineMeshResource({ L"Debug Drawer" });

#endif // _DEBUG

	void DebugDraw::Initialize(D_SERIALIZATION::Json const& settings)
	{
#ifdef _DEBUG
		CubeMeshResource = GetResource<D_GRAPHICS::StaticMeshResource>(D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::BoxMesh));
		SphereMeshResource = GetResource<D_GRAPHICS::StaticMeshResource>(D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::LowPolySphereMesh));
		LineMeshResource = GetResource<D_GRAPHICS::BatchResource>(D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::LineMesh));

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

	void DebugDraw::DrawCube(D_MATH::Vector3 const& position, D_MATH::Quaternion const& rotation, D_MATH::Vector3 const& scale, double duration, D_MATH::Color const& color)
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);

		if (DrawPending.size() >= MAX_DEBUG_DRAWS)
			return;

		auto index = (UINT)DrawPending.size();

		// Upload transform
		auto trans = D_MATH::Transform(position, rotation, scale);
		UploadTransform(trans, index);

		RENDERSETUP(CubeMeshResource);
	}

	void DebugDraw::DrawSphere(D_MATH::Vector3 const& position, float radius, double duration, D_MATH::Color const& color)
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		if (DrawPending.size() >= MAX_DEBUG_DRAWS)
			return;

		auto index = (UINT)DrawPending.size();

		auto trans = D_MATH::Transform(position, Quaternion(kIdentity), D_MATH::Vector3(kOne) * 2 * radius);
		UploadTransform(trans, index);

		RENDERSETUP(SphereMeshResource);
	}

	void DebugDraw::DrawLine(D_MATH::Vector3 const& p1, D_MATH::Vector3 const& p2, double duration, D_MATH::Color const& color)
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		if (DrawPending.size() >= MAX_DEBUG_DRAWS)
			return;

		auto index = (UINT)DrawPending.size();

		auto difference = p2 - p1;

		auto quat = Quaternion::GetShortestArcBetweenTwoVector(Vector3::Forward, p2 - p1);

		auto trans = D_MATH::Transform(p1, quat, Vector3(kOne) * D_MATH::Length(difference));

		UploadTransform(trans, index);
		RENDERSETUP_LINE(LineMeshResource);
	}

	void DebugDraw::DrawCubeLines(D_CONTAINERS::DVector<D_MATH::Vector3> const& vertices, double duration, D_MATH::Color const& color)
	{
		// Only accepting eight vertices
		D_ASSERT(vertices.size() == 8);

		auto NearLowerLeft = vertices[0];
		auto NearUpperLeft = vertices[1];
		auto NearLowerRight = vertices[2];
		auto NearUpperRight = vertices[3];
		auto FarLowerLeft = vertices[4];
		auto FarUpperLeft = vertices[5];
		auto FarLowerRight = vertices[6];
		auto FarUpperRight = vertices[7];

		// Near rect
		DrawLine(NearLowerLeft, NearLowerRight);
		DrawLine(NearLowerRight, NearUpperRight);
		DrawLine(NearUpperRight, NearUpperLeft);
		DrawLine(NearUpperLeft, NearLowerLeft);

		// Far rect
		DrawLine(FarLowerLeft, FarLowerRight);
		DrawLine(FarLowerRight, FarUpperRight);
		DrawLine(FarUpperRight, FarUpperLeft);
		DrawLine(FarUpperLeft, FarLowerLeft);

		// Connecting near plane and far plane
		DrawLine(NearUpperLeft, FarUpperLeft);
		DrawLine(NearUpperRight, FarUpperRight);
		DrawLine(NearLowerLeft, FarLowerLeft);
		DrawLine(NearLowerRight, FarLowerRight);
	}

	void DebugDraw::DrawFrustum(D_MATH_CAMERA::Frustum const& frus, double duration, D_MATH::Color const& color)
	{
		auto NearLowerLeft = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kNearLowerLeft);
		auto NearUpperLeft = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kNearUpperLeft);
		auto NearLowerRight = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kNearLowerRight);
		auto NearUpperRight = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kNearUpperRight);
		auto FarLowerLeft = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kFarLowerLeft);
		auto FarUpperLeft = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kFarUpperLeft);
		auto FarLowerRight = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kFarLowerRight);
		auto FarUpperRight = frus.GetFrustumCorner(D_MATH_CAMERA::Frustum::kFarUpperRight);

		// Near rect
		DrawLine(NearLowerLeft, NearLowerRight, duration, color);
		DrawLine(NearLowerRight, NearUpperRight, duration, color);
		DrawLine(NearUpperRight, NearUpperLeft, duration, color);
		DrawLine(NearUpperLeft, NearLowerLeft, duration, color);

		// Far rect
		DrawLine(FarLowerLeft, FarLowerRight, duration, color);
		DrawLine(FarLowerRight, FarUpperRight, duration, color);
		DrawLine(FarUpperRight, FarUpperLeft, duration, color);
		DrawLine(FarUpperLeft, FarLowerLeft, duration, color);

		// Connecting near plane and far plane
		DrawLine(NearUpperLeft, FarUpperLeft, duration, color);
		DrawLine(NearUpperRight, FarUpperRight, duration, color);
		DrawLine(NearLowerLeft, FarLowerLeft, duration, color);
		DrawLine(NearLowerRight, FarLowerRight, duration, color);
	}

	void DebugDraw::DrawConeLines(D_MATH::Vector3 const& tipLocation, D_MATH::Vector3 const& tipToBaseDirection, float height, float baseRadius, double duration, D_MATH::Color const& color)
	{
		Vector3 tipToBaseNorm = D_MATH::Normalize(tipToBaseDirection);
		Vector3 tipToBase = tipToBaseNorm * height;
		Vector3 baseCenter = tipLocation + tipToBase;
		Vector3 temp = D_MATH::Normalize(tipToBaseNorm + Vector3(0.f, 1.f, 0.f));
		if ((bool)(temp == tipToBaseNorm))
			temp = tipToBaseNorm + Vector3(1.f, 0.f, 0.f);

		Vector3 n = D_MATH::Normalize(D_MATH::Cross(temp, tipToBaseNorm)); // A normal vector on base
		Vector3 m = D_MATH::Cross(n, tipToBaseNorm); // Another normal vec on base and perpendicular to n

		for (int i = 0; i < 12; i++) // Per point on base perimeter
		{
			Vector3 pointOnBase = ((m * D_MATH::Sin(DirectX::XM_2PI * (float)i / 12.f)) + (n * D_MATH::Cos(DirectX::XM_2PI * (float)i / 12.f))) * baseRadius + baseCenter;

			DrawLine(tipLocation, pointOnBase, duration, color);
		}
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
		auto& currentUploadBuff = MeshConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];
		MeshConstants& cb = ((MeshConstants*)currentUploadBuff.Map())[index];

		auto world = trans.GetWorld();
		cb.World = Matrix4(world);
		cb.WorldIT = InverseTranspose(Matrix3(world));

		currentUploadBuff.Unmap();

	}

	void DebugDraw::FinalizeUpload()
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		if (DrawPending.size() <= 0)
			return;

		auto& currentUploadBuff = MeshConstantsCPU[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];

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
				ri.MeshVsCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER_FRAME_RESOURCE::MeshConstants) * index;
				DrawPending.push_back(ri);
				index++;
				it++;
			}
		}
	}

}
