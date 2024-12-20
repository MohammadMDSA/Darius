#pragma once

#include <Renderer/RendererCommon.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>
#include <Renderer/Resources/BatchResource.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Utils/Common.hpp>

#ifndef D_DEBUG_DRAW
#define D_DEBUG_DRAW Darius::Debug::DebugDraw
#endif

namespace Darius::Debug
{
	class DebugDraw
	{
	public:

		enum class CapsuleOrientation
		{
			AlongX,
			AlongY,
			AlongZ
		};

		static void Initialize(D_SERIALIZATION::Json const& settings);
		static void Shutdown();

#ifdef _D_EDITOR
		static bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

#ifdef _D_EDITOR
		static void FinalizeUpload();
		static void Clear(bool clearCache = false);

		static D_CONTAINERS::DVector<D_RENDERER::RenderItem> const& GetRenderItems();

		static void DrawCube(D_MATH::Vector3 const& position, D_MATH::Quaternion const& rotation, D_MATH::Vector3 const& scale, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawSphere(D_MATH::Vector3 const& position, float radius, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawLine(D_MATH::Vector3 const& p1, D_MATH::Vector3 const& p2, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawFrustum(D_MATH_CAMERA::Frustum const& frus, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });

		// Only accepting 8 vertices, order by near to out, left to right, lower to top
		static void DrawCubeLines(D_CONTAINERS::DVector<D_MATH::Vector3> const& vertices, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });

		static void DrawConeLines(D_MATH::Vector3 const& tipLocation, D_MATH::Vector3 const& tipToBaseDirection, float height, float baseRadius, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });
		
		static void DrawHemisphere(D_MATH::Vector3 const& centerLocation, D_MATH::Vector3 const& centerToTopDirection, float radius, UINT tessellation = 16, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });

		static void DrawCapsule(D_MATH::Vector3 const& centerLocation, float radius, float halfHeight, D_MATH::Quaternion const& rotation, CapsuleOrientation orientation, UINT tessellation = 16, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });

		static void DrawMesh(D_MATH::Vector3 const& position, D_MATH::Quaternion const& rotation, D_MATH::Vector3 const& scale, D_RENDERER_GEOMETRY::Mesh const* mesh, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });

#else
		static INLINE void FinalizeUpload() {}
		static INLINE void Clear(bool clearCache = false) {}

		static INLINE D_CONTAINERS::DVector<D_RENDERER::RenderItem> const& GetRenderItems() { return {}; }

		static INLINE void DrawCube(D_MATH::Vector3 const& position, D_MATH::Quaternion const& rotation, D_MATH::Vector3 const& scale, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawCube(D_MATH::Vector3 const& position, float radius, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawSphere(D_MATH::Vector3 const& position, float radius, double duration = 0.f, D_MATH::Color const& color = {1.f, 1.f, 1.f, 1.f}) {}
		static INLINE void DrawLine(D_MATH::Vector3 const& p1, D_MATH::Vector3 const& p2, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawFrustum(D_MATH_CAMERA::Frustum const& frus, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawCubeLines(D_CONTAINERS::DVector<D_MATH::Vector3> const& vertices, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawConeLines(D_MATH::Vector3 const& tipLocation, D_MATH::Vector3 const& tipToBaseDirection, float height, float baseRadius, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawHemisphere(D_MATH::Vector3 const& centerLocation, D_MATH::Vector3 const& centerToTopDirection, float radius, UINT tessellation = 16, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawCapsule(D_MATH::Vector3 const& centerLocation, float radius, float halfHeight, D_MATH::Quaternion const& rotation, CapsuleOrientation orientation, UINT tessellation = 16, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawMesh(D_MATH::Vector3 const& position, D_MATH::Quaternion const& rotation, D_MATH::Vector3 const& scale, D_RENDERER_GEOMETRY::Mesh const* mesh, double duration = 0., D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}



#endif // _D_EDITOR


	private:
#ifdef _D_EDITOR
		static void PopulateRenderItemFromMesh(D_RENDERER::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh);
		static void UploadTransform(D_MATH::Transform const& trans, UINT index);

		static D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource>	CubeMeshResource;
		static D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource>	SphereMeshResource;
		static D_RESOURCE::ResourceRef<D_RENDERER::BatchResource>		LineMeshResource;

#else
		static INLINE void PopulateRenderItemFromMesh(D_RENDERER::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh) {}
		static INLINE void UploadTransform(D_MATH::Transform const& trans, UINT index) {}

#endif // _D_EDITOR

	};
}
