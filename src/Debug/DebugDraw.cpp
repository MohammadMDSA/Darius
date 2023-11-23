#include "pch.hpp"
#include "DebugDraw.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Renderer/RendererManager.hpp>
#include <Renderer/Rasterization/Renderer.hpp>

#include <mutex>

#define MAX_DEBUG_DRAWS 1024


#define RENDERSETUP_LINE(meshResource) \
D_RENDERER::RenderItem ri; \
PopulateRenderItemFromMesh(ri, meshResource->GetMeshData()); \
ri.MeshVsCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER::MeshConstants) * index; \
ri.Color = color; \
ri.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST; \
ri.PsoFlags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::LineOnly | RenderItem::AlphaBlend; \
ri.PsoType = D_RENDERER_RAST::GetPso({ ri.PsoFlags }); \
ri.DepthPsoIndex = D_RENDERER_RAST::GetPso({ (UINT16)(ri.PsoFlags | RenderItem::DepthOnly) }); \
DrawPending.push_back(ri); \
if(duration > 0) \
{ \
	DrawsWithDuration.insert({ D_TIME::GetTotalTime() + duration, { ri, trans } }); \
}

#define RENDERSETUP(meshResource) \
D_RENDERER::RenderItem ri; \
PopulateRenderItemFromMesh(ri, meshResource->GetMeshData()); \
ri.MeshVsCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER::MeshConstants) * index; \
ri.Color = color; \
ri.PsoFlags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::Wireframe; \
ri.PsoType = D_RENDERER_RAST::GetPso({ ri.PsoFlags }); \
ri.DepthPsoIndex = D_RENDERER_RAST::GetPso({ (UINT16)(ri.PsoFlags | RenderItem::DepthOnly) }); \
DrawPending.push_back(ri); \
if(duration > 0) \
{ \
	DrawsWithDuration.insert({ D_TIME::GetTotalTime() + duration, { ri, trans } }); \
}

using namespace D_MATH;
using namespace D_RENDERER;
using namespace D_RESOURCE;

namespace Darius::Debug
{

#ifdef _DEBUG

	// Gpu buffers
	D_GRAPHICS_BUFFERS::UploadBuffer		MeshConstantsCPU;
	D_GRAPHICS_BUFFERS::ByteAddressBuffer	MeshConstantsGPU;

	std::mutex								AdditionMutex;

	D_CONTAINERS::DVector<D_RENDERER::RenderItem> DrawPending;

	D_CONTAINERS::DUnorderedMap<double, std::pair<D_RENDERER::RenderItem, D_MATH::Transform>> DrawsWithDuration;

	D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource>	DebugDraw::CubeMeshResource;
	D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource>	DebugDraw::SphereMeshResource;
	D_RESOURCE::ResourceRef<D_RENDERER::BatchResource>		DebugDraw::LineMeshResource;

#endif // _DEBUG

	void DebugDraw::Initialize(D_SERIALIZATION::Json const& settings)
	{
#ifdef _DEBUG
		CubeMeshResource = GetResourceSync<D_RENDERER::StaticMeshResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::BoxMesh));
		SphereMeshResource = GetResourceSync<D_RENDERER::StaticMeshResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::LowPolySphereMesh));
		LineMeshResource = GetResourceSync<D_RENDERER::BatchResource>(D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::LineMesh));

		// Initializing Mesh Constants buffers
		MeshConstantsCPU.Create(L"Debug Mesh Constant Upload Buffer", sizeof(D_RENDERER::MeshConstants) * MAX_DEBUG_DRAWS);
		MeshConstantsGPU.Create(L"Debug Mesh Constant GPU Buffer", MAX_DEBUG_DRAWS, sizeof(D_RENDERER::MeshConstants));

		DrawPending.reserve(500);

#endif // _DEBUG

	}

	void DebugDraw::Shutdown()
	{
		MeshConstantsCPU.Destroy();
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

		auto quat = Quaternion::GetShortestArcBetweenTwoVector(Vector3::Forward, difference);

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

	void DebugDraw::DrawHemisphere(D_MATH::Vector3 const& centerLocation, D_MATH::Vector3 const& centerToTopDirection, float radius, UINT tessellation, double duration, D_MATH::Color const& color)
	{
		D_ASSERT_M(tessellation > 3, "Tessellation parameter must be at least 3");
		const UINT verticalSegments = tessellation;
		const UINT horizontalSegments = tessellation * 2;

		using namespace DirectX;

		D_CONTAINERS::DVector<Vector3> verticesLocations;
		verticesLocations.reserve((horizontalSegments + 1) * (tessellation / 2) + 1);

		auto baseRotation = Quaternion::GetShortestArcBetweenTwoVector(Vector3::Up, centerToTopDirection.Normalize());

		// Latitude rings, starting from main belt to top
		for (UINT i = verticalSegments / 2; i <= verticalSegments; i++)
		{
			const float v = 1 - float(i) / float(verticalSegments);

			const float lat = (float(i) * XM_PI / float(verticalSegments)) - XM_PIDIV2;
			float dy, dxz;
			XMScalarSinCos(&dy, &dxz, lat);

			// Create verts of this latitude
			for (UINT j = 0; j <= horizontalSegments; j++)
			{
				const float u = float(j) / float(horizontalSegments);

				const float longtitude = float(j) * XM_2PI / float(horizontalSegments);
				float dx, dz;

				XMScalarSinCos(&dx, &dz, longtitude);

				dx *= dxz;
				dz *= dxz;

				Vector3 pos(dx, dy, dz);
				pos = (baseRotation * pos) * radius + centerLocation;

				verticesLocations.push_back(pos);
			}
		}

		// Drawing vertical lines
		const UINT stride = horizontalSegments + 1;

		for (UINT i = 0u; i < verticalSegments / 2; i++)
		{
			for (UINT j = 0; j < 4; j++)
			{
				UINT horiz = (horizontalSegments * j) / 4;
				Vector3& p1 = verticesLocations[i * stride + horiz];
				Vector3& p2 = verticesLocations[(i + 1) * stride + horiz];
				DrawLine(p1, p2, duration, color);
			}
		}

		// Draing main belt
		for (UINT i = 0; i < horizontalSegments - 1; i++)
		{
			Vector3& p1 = verticesLocations[i];
			Vector3& p2 = verticesLocations[i + 1];
			DrawLine(p1, p2, duration, color);
		}
		DrawLine(verticesLocations[horizontalSegments - 1], verticesLocations[0], duration, color);
	}

	void DebugDraw::DrawCapsule(D_MATH::Vector3 const& centerLocation, float radius, float halfHeight, D_MATH::Quaternion const& rotation, CapsuleOrientation orientation, UINT tessellation, double duration, D_MATH::Color const& color)
	{
		Vector3 centerToTipDirection;
		switch (orientation)
		{
		case Darius::Debug::DebugDraw::CapsuleOrientation::AlongX:
			centerToTipDirection = rotation * Vector3(1.f, 0.f, 0.f);
			break;
		case Darius::Debug::DebugDraw::CapsuleOrientation::AlongY:
			centerToTipDirection = rotation * Vector3(0.f, 1.f, 0.f);
			break;
		case Darius::Debug::DebugDraw::CapsuleOrientation::AlongZ:
			centerToTipDirection = rotation * Vector3(0.f, 0.f, 1.f);
			break;
		default:
			D_ASSERT(false);
			break;
		}
		
		// Drawing two hemisphere at its two ends
		DrawHemisphere(centerToTipDirection * halfHeight + centerLocation, centerToTipDirection, radius, tessellation, duration, color);
		DrawHemisphere(-centerToTipDirection * halfHeight + centerLocation, -centerToTipDirection, radius, tessellation, duration, color);
	}

	void DebugDraw::DrawConeLines(D_MATH::Vector3 const& tipLocation, D_MATH::Vector3 const& tipToBaseDirection, float height, float baseRadius, double duration, D_MATH::Color const& color)
	{
#define ConeBasePoints 12

		Vector3 tipToBaseNorm = D_MATH::Normalize(tipToBaseDirection);
		Vector3 tipToBase = tipToBaseNorm * height;
		Vector3 baseCenter = tipLocation + tipToBase;
		Vector3 temp = D_MATH::Normalize(tipToBaseNorm + Vector3(0.f, 1.f, 0.f));
		if ((bool)(temp == tipToBaseNorm))
			temp = tipToBaseNorm + Vector3(1.f, 0.f, 0.f);

		Vector3 n = D_MATH::Normalize(D_MATH::Cross(temp, tipToBaseNorm)); // A normal vector on base
		Vector3 m = D_MATH::Cross(n, tipToBaseNorm); // Another normal vec on base and perpendicular to n

		Vector3 vec[ConeBasePoints];

		for (int i = 0; i < ConeBasePoints; i++) // Per point on base perimeter
		{
			vec[i] = ((m * D_MATH::Sin(DirectX::XM_2PI * (float)i / 12.f)) + (n * D_MATH::Cos(DirectX::XM_2PI * (float)i / ConeBasePoints))) * baseRadius + baseCenter;
		}

		for (int i = 0; i < ConeBasePoints; i++)
		{
			DrawLine(tipLocation, vec[i], duration, color);
			DrawLine(vec[(i + 1) % ConeBasePoints], vec[i], duration, color);
		}
	}

	void DebugDraw::PopulateRenderItemFromMesh(D_RENDERER::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh)
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
		MeshConstants& cb = ((MeshConstants*)MeshConstantsCPU.Map())[index];

		auto world = trans.GetWorld();
		cb.World = Matrix4(world);
		cb.WorldIT = InverseTranspose(Matrix3(world));

		MeshConstantsCPU.Unmap();

	}

	void DebugDraw::FinalizeUpload()
	{
		const std::lock_guard<std::mutex> lock(AdditionMutex);
		if (DrawPending.size() <= 0)
			return;

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Debug Mesh CBV Upload");
		context.TransitionResource(MeshConstantsGPU, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(MeshConstantsGPU.GetResource(), 0, MeshConstantsCPU.GetResource(), 0, sizeof(D_RENDERER::MeshConstants) * DrawPending.size());
		context.TransitionResource(MeshConstantsGPU, D3D12_RESOURCE_STATE_GENERIC_READ);

		context.Finish();
	}

	D_CONTAINERS::DVector<D_RENDERER::RenderItem> const& DebugDraw::GetRenderItems()
	{
		//D_LOG_DEBUG(DrawPending.size());
		const std::lock_guard<std::mutex> lock(AdditionMutex);

		return DrawPending;
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
				ri.MeshVsCBV = MeshConstantsGPU.GetGpuVirtualAddress() + sizeof(D_RENDERER::MeshConstants) * index;
				DrawPending.push_back(ri);
				index++;
				it++;
			}
		}
	}

}
