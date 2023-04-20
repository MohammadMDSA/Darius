#pragma once

#include <Renderer/FrameResource.hpp>
#include <Renderer/Renderer.hpp>
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

		static void Initialize(D_SERIALIZATION::Json const& settings);
		static void Shutdown();

#ifdef _D_EDITOR
		static bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

#ifdef _DEBUG
		static void FinalizeUpload();
		static void Clear(bool clearCache = false);

		static void GetRenderItems(D_RENDERER::MeshSorter& sorter);

		static void DrawCube(D_MATH::Vector3 const& position, D_MATH::Quaternion const& rotation, D_MATH::Vector3 const& scale, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawSphere(D_MATH::Vector3 const& position, float radius, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawLine(D_MATH::Vector3 const& p1, D_MATH::Vector3 const& p2, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });
		static void DrawFrustum(D_MATH_CAMERA::Frustum const& frus, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f });

#else
		static INLINE void FinalizeUpload() {}
		static INLINE void Clear(bool clearCache = false) {}

		static INLINE void GetRenderItems(D_RENDERER::MeshSorter& sorter) {}

		static INLINE void DrawCube(D_MATH::Vector3 const& position, D_MATH::Quaternion const& rotation, D_MATH::Vector3 const& scale, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawCube(D_MATH::Vector3 const& position, float radius, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawLine(D_MATH::Vector3 const& p1, D_MATH::Vector3 const& p2, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}
		static INLINE void DrawFrustum(D_MATH_CAMERA::Frustum const& frus, double duration = 0.f, D_MATH::Color const& color = { 1.f, 1.f, 1.f, 1.f }) {}


#endif // _DEBUG


	private:
#ifdef _DEBUG
		static void PopulateRenderItemFromMesh(D_RENDERER_FRAME_RESOURCE::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh);
		static void UploadTransform(D_MATH::Transform const& trans, UINT index);

		static D_CORE::Ref<D_GRAPHICS::StaticMeshResource>	CubeMeshResource;
		static D_CORE::Ref<D_GRAPHICS::StaticMeshResource>	SphereMeshResource;
		static D_CORE::Ref<D_GRAPHICS::BatchResource>		LineMeshResource;

#else
		static INLINE void PopulateRenderItemFromMesh(D_RENDERER_FRAME_RESOURCE::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh) {}
		static INLINE void UploadTransform(D_MATH::Transform const& trans, UINT index) {}

#endif // _DEBUG

	};
}
