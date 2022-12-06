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

		void							Create(MultiPartMeshData<VertexType>& data);

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		D_CH_R_FIELD(DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>, Skeleton);
		D_CH_R_FIELD(D_RENDERER_GEOMETRY::Mesh::SkeletonJoint*, SkeletonRoot);
		D_CH_R_FIELD(int, JointCount);

	protected:

		SkeletalMeshResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, name, id, isDefault),
			mJointCount(0),
			mSkeletonRoot(nullptr) {}

		virtual bool					UploadToGpu(void* context) override;

	private:
		static void						GetFBXSkin(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DList<Mesh::SkeletonJoint>& skeletonHierarchy, DVector<DUnorderedMap<int, int>>& indexMapper);
		static void						ReadFBXCacheVertexPositions(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper);
		static void						AddSkeletonChildren(void const* skeletonNode, DList<Mesh::SkeletonJoint>& skeletonData, DMap<void const*, int>& skeletonIndexMap);
		static void						AddJointWeightToVertices(MultiPartMeshData<VertexType>& meshDataVec,
			VertexBlendWeightData& skinData, DVector<DUnorderedMap<int, int>> const& indexMapper);
		static void						AddBlendDataToVertex(MeshResource::VertexType& vertex, DVector<std::pair<int, std::pair<float, D_MATH::Matrix4>>>& blendData);

	};
}
