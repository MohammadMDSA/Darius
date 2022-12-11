#pragma once

#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Geometry/MeshData.hpp"
#include "Renderer/GraphicsUtils/VertexTypes.hpp"

#include <Core/Containers/Vector.hpp>
#include <ResourceManager/Resource.hpp>

#ifndef D_RENDERER_GEOMETRY_LOADER_FBX
#define D_RENDERER_GEOMETRY_LOADER_FBX Darius::Renderer::Geometry::ModelLoader::Fbx
#endif

namespace Darius::Renderer::Geometry::ModelLoader::Fbx
{

	D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> GetResourcesDataFromFile(D_RESOURCE::ResourceType type, D_FILE::Path const& path);

	bool ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result);

	bool ReadMeshByName(D_FILE::Path const& path, std::wstring const& meshName, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result, DList<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& skeleton);

}