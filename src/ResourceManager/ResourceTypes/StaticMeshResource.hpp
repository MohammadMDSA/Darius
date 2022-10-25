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
		void							Create(MultiPartMeshData<VertexType>& data);

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { (params); return false; };
#endif // _D_EDITOR

		INLINE operator const Mesh* () const { return &mMesh; }
		INLINE operator Mesh* () { return ModifyMeshData(); }

	protected:
		
		StaticMeshResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, id, isDefault) {}

		virtual bool					UploadToGpu(D_GRAPHICS::GraphicsContext& context) override;

		static bool						CanConstructFrom(Path const&);

	private:
		friend class DResourceManager;
	};
}