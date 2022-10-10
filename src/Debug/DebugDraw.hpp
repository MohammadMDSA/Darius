#pragma once

#include <Renderer/FrameResource.hpp>
#include <Renderer/Renderer.hpp>
#include <ResourceManager/ResourceTypes/MeshResource.hpp>
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

		static void Initialize();
		static void Shutdown();


#ifdef _DEBUG
		static void FinalizeUpload();
		static void Clear();

		static void GetRenderItems(D_RENDERER::MeshSorter& sorter);

		static void DrawCube(D_MATH::Vector3 position, D_MATH::Quaternion rotation, D_MATH::Vector3 scale, D_MATH::Color color = { 1.f, 1.f, 1.f, 1.f });
#else
		static INLINE void FinalizeUpload() {}
		static INLINE void Clear() {}

		static INLINE void GetRenderItems(D_RENDERER::MeshSorter& sorter) {}

		static INLINE void DrawCube(D_MATH::Vector3 position, D_MATH::Quaternion rotation, D_MATH::Vector3 scale, D_MATH::Color color = { 1.f, 1.f, 1.f, 1.f }) {}

#endif // _DEBUG


	private:
#ifdef _DEBUG
		static void PopulateRenderItemFromMesh(D_RENDERER_FRAME_RESOUCE::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh);
		static void UploadTransform(D_MATH::Transform const& trans, UINT index);
#else
		static INLINE void PopulateRenderItemFromMesh(D_RENDERER_FRAME_RESOUCE::RenderItem& renderItem, D_RENDERER_GEOMETRY::Mesh const* mesh) {}
		static INLINE void UploadTransform(D_MATH::Transform const& trans, UINT index) {}

#endif // _DEBUG

	};
}
