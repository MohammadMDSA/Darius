#pragma once

#include "MeshResource.hpp"

#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Geometry/MeshData.hpp"
#include "Renderer/VertexTypes.hpp"

#include "StaticMeshResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	class DClass(Serialize, Resource) StaticMeshResource : public MeshResource
	{
		GENERATED_BODY();

	public:
		D_CH_RESOURCE_BODY(StaticMeshResource, "Static Mesh", ".fbx")


	public:
		INLINE D_RENDERER_GEOMETRY::Mesh*		ModifyMeshData() { MakeDiskDirty(); MakeGpuDirty(); return &mMesh; }
		INLINE const D_RENDERER_GEOMETRY::Mesh*	GetMeshData() const { return &mMesh; }
#ifdef _D_EDITOR
		virtual bool							DrawDetails(float params[]);
#endif // _D_EDITOR

		INLINE operator const D_RENDERER_GEOMETRY::Mesh* () const { return &mMesh; }
		INLINE operator D_RENDERER_GEOMETRY::Mesh* () { return ModifyMeshData(); }

	protected:
		
		StaticMeshResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, name, id, isDefault) {}

		virtual void							CreateInternal(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data) override;

	private:
		friend class DResourceManager;


	};
}

File_StaticMeshResource_GENERATED
