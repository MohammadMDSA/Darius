#pragma once

#include "Renderer/GraphicsUtils/VertexTypes.hpp"

#include <ResourceManager/Resource.hpp>
#include <Utils/Assert.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

using namespace D_RENDERER_GEOMETRY;
using namespace D_CORE;

using namespace D_RESOURCE;

namespace Darius::Graphics
{
	class DResourceManager;

	class MeshResource : public Resource
	{
	public:
		using VertexType = D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned;
	public:
		INLINE Mesh*					ModifyMeshData() { MakeDiskDirty(); MakeGpuDirty(); return &mMesh; }
		INLINE const Mesh*				GetMeshData() const { return &mMesh; }
		virtual void					Create(MultiPartMeshData<VertexType> const& data);

		static DVector<ResourceDataInFile> CanConstructFrom(ResourceType type, Path const& path);

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { (params); return false; };
#endif // _D_EDITOR

		INLINE operator const Mesh* () const { return &mMesh; }
		INLINE operator Mesh* () { return ModifyMeshData(); }

		D_CH_FIELD_ACC(Mesh, Mesh, protected);

	protected:

		MeshResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}

		INLINE virtual void				WriteResourceToFile(D_SERIALIZATION::Json&) const override {};
		INLINE virtual void				ReadResourceFromFile(D_SERIALIZATION::Json const&) override {};
		virtual bool					UploadToGpu(void* context) override;

		INLINE virtual void				Unload() override { EvictFromGpu(); }

		
		friend class DResourceManager;

	};
}