#pragma once

#include "MeshResource.hpp"

#include "Renderer/Geometry/MeshData.hpp"
#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/GraphicsUtils/VertexTypes.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

using namespace D_RENDERER_GEOMETRY;
using namespace D_CORE;

namespace Darius::Graphics
{
	class DResourceManager;

	class StaticMeshResource : public MeshResource
	{
	public:
		D_CH_RESOURCE_BODY(StaticMeshResource, "Static Mesh", ".fbx")


	public:
		INLINE Mesh*					ModifyMeshData() { MakeDiskDirty(); MakeGpuDirty(); return &mMesh; }
		INLINE const Mesh*				GetMeshData() const { return &mMesh; }
		virtual void					Create(MultiPartMeshData<VertexType> const& data) override;

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { (params); return false; };
#endif // _D_EDITOR

		INLINE operator const Mesh* () const { return &mMesh; }
		INLINE operator Mesh* () { return ModifyMeshData(); }

	protected:
		
		StaticMeshResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, name, id, isDefault) {}
	private:
		friend class DResourceManager;
	};
}