#include "Renderer/pch.hpp"
#include "MeshResource.hpp"

#include "SkeletalMeshResource.hpp"
#include "Renderer/GraphicsDeviceManager.hpp"
#include "Renderer/GraphicsCore.hpp"
#include "Renderer/Geometry/ModelLoader/FbxLoader.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Set.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include "MeshResource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;

namespace Darius::Graphics
{

	DVector<ResourceDataInFile> MeshResource::CanConstructFrom(ResourceType type, Path const& path)
	{
		return D_RENDERER_GEOMETRY_LOADER_FBX::GetMeshResourcesDataFromFile(type, path);
	}

	bool MeshResource::UploadToGpu()
	{
		MultiPartMeshData<VertexType> meshData;

		D_RENDERER_GEOMETRY_LOADER_FBX::ReadMeshByName(GetPath(), GetName(), meshData);

		Create(meshData);
		return true;
	}

}