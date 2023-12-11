#include "Renderer/pch.hpp"
#include "MeshResource.hpp"

#include "SkeletalMeshResource.hpp"
#include "Renderer/Geometry/ModelLoader/FbxLoader.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Set.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "MeshResource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;

namespace Darius::Renderer
{

	DVector<ResourceDataInFile> MeshResource::CanConstructFrom(ResourceType type, Path const& path)
	{
		return D_RENDERER_GEOMETRY_LOADER_FBX::GetMeshResourcesDataFromFile(type, path);
	}

	void MeshResource::Create(D_RENDERER_GEOMETRY::MultiPartMeshData<VertexType> const& data)
	{
		CreateInternal(data);

		MakeDiskDirty();
		MakeGpuDirty();

		SignalChange();
	}

	bool MeshResource::UploadToGpu()
	{
		if (IsDefault())
			return true;

		MultiPartMeshData<VertexType> meshData;

		D_RENDERER_GEOMETRY_LOADER_FBX::ReadMeshByName(GetPath(), GetName(), meshData);

		CreateInternal(meshData);
		return true;
	}

}