#pragma once

#include "MaterialResource.hpp"
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
		GENERATED_BODY();
		D_CH_RESOURCE_ABSTRACT_BODY(MeshResource)

	public:
		using VertexType = D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned;

		enum class DEnum(Serialize) NormalsReordering
		{
			XYZ,
			XZY,
			YXZ,
			YZX,
			ZXY,
			ZYX
		};

	public:
		INLINE D_RENDERER_GEOMETRY::Mesh*	ModifyMeshData() { MakeDiskDirty(); MakeGpuDirty(); return &mMesh; }
		INLINE D_RENDERER_GEOMETRY::Mesh const*	GetMeshData() const { return &mMesh; }
		virtual void						Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data);

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		INLINE operator const D_RENDERER_GEOMETRY::Mesh* () const { return &mMesh; }
		INLINE operator D_RENDERER_GEOMETRY::Mesh* () { return ModifyMeshData(); }

		INLINE virtual bool				AreDependenciesDirty() const override { return false; }

		INLINE bool						IsInverted() const { return mInverted; }
		void							SetInverted(bool value);

		INLINE NormalsReordering		GetNormalsReordering() const { return mNormalsReordering; }
		void							SetNormalsReordering(NormalsReordering order);

		INLINE MaterialResource* GetMaterial(int index) const {
			D_ASSERT(index >= 0 && index < (int)mMaterials.size()); return mMaterials.at(index).Get(); }
		INLINE D_CONTAINERS::DVector<D_RESOURCE::ResourceRef<MaterialResource>> const& GetMaterials() const { return mMaterials; }
		void							SetMaterial(int index, MaterialResource* material);
	protected:

		MeshResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault),
			mNormalsReordering(NormalsReordering::XYZ),
			mInverted(false)
		{}

		virtual void					CreateInternal(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data) = 0;

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json&) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const&, bool& dirtyDisk) override;

		INLINE virtual void				Unload() override { EvictFromGpu(); }
		INLINE virtual bool				UploadToGpu() override { return true; }

		void							SetMaterialListSize(UINT size);

		D_RENDERER_GEOMETRY::Mesh		mMesh;
		
	private:

		DField(Serialize)
		bool							mInverted;

		DField(Serialize)
		NormalsReordering				mNormalsReordering;

		DField(Serialize)
		D_CONTAINERS::DVector<D_RESOURCE::ResourceRef<MaterialResource>> mMaterials;

		friend class DResourceManager;
	};
}

File_MeshResource_GENERATED
