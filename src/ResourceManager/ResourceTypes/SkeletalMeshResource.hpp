#pragma once

#include "MeshResource.hpp"

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

		void							Create(MultiPartMeshData<VertexType>& data, DVector<D_RENDERER_GEOMETRY::Mesh::SceneGraphNode>& skeleton);

		D_CH_R_FIELD(DVector<D_RENDERER_GEOMETRY::Mesh::SceneGraphNode>, Skeleton);
		D_CH_R_FIELD(int, JointCount);
		D_CH_R_FIELD(DVector<Matrix4>, IBMatrices);

	protected:

		SkeletalMeshResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, id, isDefault),
			mJointCount(0) {}

		virtual bool					UploadToGpu(D_GRAPHICS::GraphicsContext& context) override;

		static bool						CanConstructFrom(Path const& path);

	private:
		static void						GetFBXSkin(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<Mesh::SceneGraphNode>& skeletonHierarchy, DVector<Matrix4>& ibms, DVector<DUnorderedMap<int, int>>& indexMapper);
		static void						AddSkeletonChildren(void const* skeletonNode, DVector<Mesh::SceneGraphNode>& skeletonData, DMap<void const*, int>& skeletonIndexMap);
		static void						AddJointWeightToVertices(MultiPartMeshData<VertexType>& meshDataVec,
			VertexBlendWeightData& skinData, DVector<DUnorderedMap<int, int>> const& indexMapper);
		static void						AddBlendDataToVertex(MeshResource::VertexType& vertex, DVector<std::pair<int, float>>& blendData);
	};
}
