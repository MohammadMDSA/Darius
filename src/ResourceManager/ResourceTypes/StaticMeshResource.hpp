#pragma once

#include "MeshResource.hpp"

#include <Renderer/Geometry/MeshData.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/GraphicsUtils/VertexTypes.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_RENDERER_GEOMETRY;
using namespace D_CORE;

namespace Darius::ResourceManager
{
	class DResourceManager;

	class StaticMeshResource : public MeshResource
	{
	public:
		D_CH_RESOURCE_BODY(StaticMeshResource, "Static Mesh", ".fbx")


	public:
		INLINE Mesh*					ModifyMeshData() { MakeDiskDirty(); MakeGpuDirty(); return &mMesh; }
		INLINE const Mesh*				GetMeshData() const { return &mMesh; }
		virtual void					Create(D_CONTAINERS::DVector<MeshData<VertexType>>& data) override;

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { (params); return false; };
#endif // _D_EDITOR

		INLINE operator const Mesh* () const { return &mMesh; }
		INLINE operator Mesh* () { return ModifyMeshData(); }

		D_CH_FIELD_ACC(Mesh, Mesh, protected);

	protected:

		StaticMeshResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, id, isDefault) {}

		static bool						CanConstructFrom(Path const&);

	private:
		friend class DResourceManager;

		void GetFBXUVs(DVector<D_RENDERER_GEOMETRY::MeshData<VertexType>>& meshData, void const* mesh, DVector<DUnorderedMap<int, int>>& indexMapper);
		void GetFBXNormalss(DVector<D_RENDERER_GEOMETRY::MeshData<VertexType>>& meshData, void const* mesh, DVector<DUnorderedMap<int, int>>& indexMapper);
		
	};
}