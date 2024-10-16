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
		GENERATED_BODY();

		D_CH_RESOURCE_BODY(SkeletalMeshResource, "Skeletal Mesh", "")
	public:


#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		INLINE D_RENDERER_GEOMETRY::Mesh::SkeletonJoint const* GetSkeletonRoot() const { return mSkeletonRoot; }
		INLINE D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> const& GetSkeleton() const { return mSkeleton; }
		
		virtual void					Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data, D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> const& skeleton);

	private:
		DField()
		D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> mSkeleton;
		
		DField()
		D_RENDERER_GEOMETRY::Mesh::SkeletonJoint*	mSkeletonRoot;
		
		DField(Get[inline])
		UINT										mJointCount;

	protected:

		SkeletalMeshResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false) :
			MeshResource(uuid, path, name, id, parent, isDefault),
			mJointCount(0),
			mSkeletonRoot(nullptr) {}

		virtual void					CreateInternal(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data) override;
		virtual void					MakeVertexList(D_CONTAINERS::DVector<VertexType> const& inputVerts, D_CONTAINERS::DVector<D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned>& outVertices) const;

	};
}

File_SkeletalMeshResource_GENERATED