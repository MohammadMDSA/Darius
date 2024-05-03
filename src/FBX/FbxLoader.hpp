#pragma once

#include <Renderer/Geometry/Mesh.hpp>
#include <Renderer/Geometry/MeshData.hpp>
#include <Renderer/VertexTypes.hpp>
#include <Renderer/Resources/MeshResource.hpp>

#include <Core/Containers/Vector.hpp>
#include <Core/Containers/List.hpp>
#include <ResourceManager/Resource.hpp>

#ifndef D_FBX
#define D_FBX Darius::Fbx
#endif // !D_FBX

namespace Darius::Fbx
{
	D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> GetMeshResourcesDataFromFile(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

	D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> GetResourcesDataFromFile(D_FILE::Path const& path);

	bool		LoadSubResources(D_FILE::Path const& path, D_RESOURCE::Resource* parentResource);

	bool		ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, D_RENDERER::MeshResource::MeshImportConfig const& importConfig, D_RENDERER_GEOMETRY::MultiPartMeshData<D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned>& result);

	bool		ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, D_RENDERER::MeshResource::MeshImportConfig const& importConfig, D_RENDERER_GEOMETRY::MultiPartMeshData<D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned>& result, D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& skeleton);

}