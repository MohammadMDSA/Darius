#pragma once

#include "MeshResource.hpp"

#include "Renderer/Geometry/MeshData.hpp"
#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/GraphicsUtils/VertexTypes.hpp"

#include <Core/Containers/List.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DResourceManager;

	class SkeletalMeshResource : public MeshResource
	{
	public:
		D_CH_RESOURCE_BODY(SkeletalMeshResource, "Skeletal Mesh", ".fbx")

		virtual void					Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data) override;

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		D_CH_R_FIELD(D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>, Skeleton);
		D_CH_R_FIELD(D_RENDERER_GEOMETRY::Mesh::SkeletonJoint*, SkeletonRoot);
		D_CH_R_FIELD(int, JointCount);

	protected:

		SkeletalMeshResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, name, id, isDefault),
			mJointCount(0),
			mSkeletonRoot(nullptr) {}

		virtual bool					UploadToGpu(void* context) override;

	};
}
