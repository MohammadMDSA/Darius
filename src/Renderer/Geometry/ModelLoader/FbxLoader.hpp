#pragma once

#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Geometry/MeshData.hpp"
#include "Renderer/GraphicsUtils/VertexTypes.hpp"

#include <Core/Containers/Vector.hpp>
#include <ResourceManager/Resource.hpp>

#ifndef D_RENDERER_GEOMETRY_LOADER
#define D_RENDERER_GEOMETRY_LOADER Darius::Renderer::Geometry::ModelLoader
#endif

namespace Darius::Renderer::Geometry::ModelLoader
{

	D_CONTAINERS::DVector<D_RESOURCE::ResourceDataInFile> GetResourcesDataFromFile(D_RESOURCE::ResourceType, D_FILE::Path const& path);

	bool ReadMeshByName(std::wstring const& meshName, MultiPartMeshData<D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned>& result);

}