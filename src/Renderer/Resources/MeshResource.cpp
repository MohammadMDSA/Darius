#include "Renderer/pch.hpp"
#include "MeshResource.hpp"

#include "SkeletalMeshResource.hpp"
#include "Renderer/Geometry/ModelLoader/FbxLoader.hpp"

#include <Core/Serialization/TypeSerializer.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Set.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#if _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>

#include <imgui.h>
#endif

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

		D_RENDERER_GEOMETRY_LOADER_FBX::ReadMeshByName(GetPath(), GetName(), IsInverted(), meshData);

		CreateInternal(meshData);
		return true;
	}

	void MeshResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		D_SERIALIZATION::Serialize(*this, json);
	};

	void MeshResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json, bool& dirtyDisk)
	{

		D_SERIALIZATION::Deserialize(*this, json);

		dirtyDisk = false;
	};

	void MeshResource::SetInverted(bool value)
	{
		if (mInverted == value)
			return;

		mInverted = value;

		MakeGpuDirty();
		MakeDiskDirty();

		SignalChange();
	}

	bool MeshResource::DrawDetails(float params[])
	{
		bool valueChanged = false;
		D_H_DETAILS_DRAW_BEGIN_TABLE()

		// Inverted
		{
			D_H_DETAILS_DRAW_PROPERTY("Inverted");
			bool value = IsInverted();
			if (ImGui::Checkbox("##Inverted", &value))
			{
				SetInverted(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	};

}