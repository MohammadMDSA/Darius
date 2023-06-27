#pragma once

#include "MeshResource.hpp"

#include "Renderer/Geometry/MeshData.hpp"
#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/VertexTypes.hpp"

#include <Core/Containers/List.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include "SkeletalMeshResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DResourceManager;

	class DClass(Serialize, Resource) SkeletalMeshResource : public MeshResource
	{
	public:
		D_CH_RESOURCE_BODY(SkeletalMeshResource, "Skeletal Mesh", ".fbx")

		virtual void					Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data) override;

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

	private:
		DField(Get[inline, const])
		D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> mSkeleton;
		
		DField(Get[inline, const])
		D_RENDERER_GEOMETRY::Mesh::SkeletonJoint*	mSkeletonRoot;
		
		DField(Get[inline])
		UINT										mJointCount;

	protected:

		SkeletalMeshResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, name, id, isDefault),
			mJointCount(0),
			mSkeletonRoot(nullptr) {}

		virtual bool					UploadToGpu() override;


	public:
		Darius_Renderer_SkeletalMeshResource_GENERATED
	};
}

File_SkeletalMeshResource_GENERATED