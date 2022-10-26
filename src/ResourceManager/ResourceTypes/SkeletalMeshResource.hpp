#pragma once

#include "MeshResource.hpp"

#include <Core/Containers/List.hpp>
#include <Renderer/Geometry/MeshData.hpp>
#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/GraphicsUtils/VertexTypes.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

namespace Darius::ResourceManager
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

		SkeletalMeshResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, id, isDefault),
			mJointCount(0),
			mSkeletonRoot(nullptr) {}

		virtual bool					UploadToGpu(D_GRAPHICS::GraphicsContext& context) override;

		static bool						CanConstructFrom(Path const& path);

	private:
		static void						GetFBXSkin(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DList<Mesh::SkeletonJoint>& skeletonHierarchy, DVector<DUnorderedMap<int, int>>& indexMapper);
		static void						ReadFBXCacheVertexPositions(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper);
		static void						AddSkeletonChildren(void const* skeletonNode, DList<Mesh::SkeletonJoint>& skeletonData, DMap<void const*, int>& skeletonIndexMap);
		static void						AddJointWeightToVertices(MultiPartMeshData<VertexType>& meshDataVec,
			VertexBlendWeightData& skinData, DVector<DUnorderedMap<int, int>> const& indexMapper);
		static void						AddBlendDataToVertex(MeshResource::VertexType& vertex, DVector<std::pair<int, float>>& blendData, Matrix4 const& initial);

	};
}
