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

		void							Create(MultiPartMeshData<VertexType>& data, D_RENDERER_GEOMETRY::Mesh::SceneGraphNode* skeleton);

	protected:

		SkeletalMeshResource(Uuid uuid, std::wstring const& path, DResourceId id, bool isDefault = false) :
			MeshResource(uuid, path, id, isDefault) {}

		virtual bool					UploadToGpu(D_GRAPHICS::GraphicsContext& context) override;

		static bool						CanConstructFrom(Path const& path);

	private:
		static void						GetFBXSkin(MultiPartMeshData<VertexType>& meshDataVec, void const* meshP, DVector<DUnorderedMap<int, int>>& indexMapper);
		static void						AddSkeletonChildren(void const* skeletonNode, DVector<Mesh::SceneGraphNode>& skeletonData, DMap<void const*, int>& skeletonIndexMap);
	};
}
