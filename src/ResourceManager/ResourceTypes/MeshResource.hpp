#pragma once

#include "Resource.hpp"

#include <Renderer/Geometry/MeshData.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/GraphicsUtils/VertexTypes.hpp>
#include <Utils/Assert.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_RENDERER_GEOMETRY;
using namespace D_CORE;

namespace Darius::ResourceManager
{
	class DResourceManager;

	class MeshResource : public Resource
	{
	public:
		using VertexType = D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned;
	public:
		INLINE Mesh*					ModifyMeshData() { MakeDiskDirty(); MakeGpuDirty(); return &mMesh; }
		INLINE const Mesh*				GetMeshData() const { return &mMesh; }

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { (params); return false; };
#endif // _D_EDITOR

		INLINE operator const Mesh* () const { return &mMesh; }
		INLINE operator Mesh* () { return ModifyMeshData(); }

		D_CH_FIELD_ACC(Mesh, Mesh, protected);

	protected:

		MeshResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, id, isDefault) {}

		INLINE virtual void				WriteResourceToFile() const override {};
		INLINE virtual void				ReadResourceFromFile() override {};
		
		friend class DResourceManager;

		static void						GetFBXUVs(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper);

		static void						GetFBXNormals(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper);

		static void						GetFBXPolygons(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper);

	};
}