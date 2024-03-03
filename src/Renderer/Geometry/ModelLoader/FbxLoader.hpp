#pragma once

#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Geometry/MeshData.hpp"
#include "Renderer/VertexTypes.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/Containers/List.hpp>
#include <Scene/GameObject.hpp>
#include <ResourceManager/Resource.hpp>

#ifndef D_RENDERER_GEOMETRY_LOADER_FBX
#define D_RENDERER_GEOMETRY_LOADER_FBX Darius::Renderer::Geometry::ModelLoader::Fbx
#endif

namespace Darius::Renderer::Geometry::ModelLoader::Fbx
{

	D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile>	GetMeshResourcesDataFromFile(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

	bool													ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, bool inverted, MultiPartMeshData<D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned>& result);

	bool													ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, bool inverted, MultiPartMeshData<D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned>& result, D_CONTAINERS::DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& skeleton);

	D_SCENE::GameObject*									LoadScene(D_FILE::Path const& path, D_CORE::Uuid const& rootUuid);

}