#pragma once

#include "Renderer/VertexTypes.hpp"
#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Geometry/MeshData.hpp"

#include <ResourceManager/Resource.hpp>
#include <Utils/Assert.hpp>

#include "MeshResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	class DClass(Serialize, Resource) MeshResource : public D_RESOURCE::Resource
	{
	public:
		using VertexType = D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned;
	public:
		INLINE D_RENDERER_GEOMETRY::Mesh*	ModifyMeshData() { MakeDiskDirty(); MakeGpuDirty(); return &mMesh; }
		INLINE const D_RENDERER_GEOMETRY::Mesh*	GetMeshData() const { return &mMesh; }
		virtual void						Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data) = 0;

		static D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> CanConstructFrom(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { (params); return false; };
#endif // _D_EDITOR

		INLINE operator const D_RENDERER_GEOMETRY::Mesh* () const { return &mMesh; }
		INLINE operator D_RENDERER_GEOMETRY::Mesh* () { return ModifyMeshData(); }


	protected:

		MeshResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}

		INLINE virtual void				WriteResourceToFile(D_SERIALIZATION::Json&) const override {};
		INLINE virtual void				ReadResourceFromFile(D_SERIALIZATION::Json const&) override {};
		virtual bool					UploadToGpu() override;

		INLINE virtual void				Unload() override { EvictFromGpu(); }

		D_RENDERER_GEOMETRY::Mesh		mMesh;
		
		friend class DResourceManager;

	public:
		Darius_Renderer_MeshResource_GENERATED
	};
}

File_MeshResource_GENERATED
