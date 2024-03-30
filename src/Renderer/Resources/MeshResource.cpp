#include "Renderer/pch.hpp"
#include "MeshResource.hpp"

#include "MaterialResource.hpp"
#include "SkeletalMeshResource.hpp"
#include "Renderer/Geometry/ModelLoader/FbxLoader.hpp"
#include "Renderer/RendererManager.hpp"

#include <Core/Serialization/TypeSerializer.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Set.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#if _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>
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
		mMaterials.resize(D_MATH::Max((int)data.SubMeshes.size(), 1));

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

		mMaterials.resize(D_MATH::Max((int)meshData.SubMeshes.size(), 1));

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

		// Materials
		{
			for (int i = 0; i < (int)mMaterials.size(); i++)
			{
				auto setter = [&, i](MaterialResource* resource)
					{
						SetMaterial(i, resource);
					};

				auto name = std::string("Material ") + std::to_string(i + 1);
				D_H_DETAILS_DRAW_PROPERTY(name.c_str());
				D_H_RESOURCE_SELECTION_DRAW(MaterialResource, mMaterials[i], "Select Material", setter, std::to_string(i));
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;
	};

	void MeshResource::SetMaterial(int index, MaterialResource* material)
	{
		D_ASSERT(index >= 0);
		if (index >= mMaterials.size())
			return;

		if (mMaterials.at(index).Get() == material)
			return;

		// What material to set?
		MaterialResource* materialToSet;
		if (material == nullptr)
			materialToSet = D_RESOURCE::GetResourceSync<MaterialResource>(GetDefaultGraphicsResource(DefaultResource::Material)).Get();
		else
			materialToSet = material;

		D_ASSERT(materialToSet);
		mMaterials[index] = materialToSet;

		if (!materialToSet->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(materialToSet, nullptr);

		// No gpu dirty since it only concerns the mesh state
		MakeDiskDirty();

		SignalChange();
	}
}