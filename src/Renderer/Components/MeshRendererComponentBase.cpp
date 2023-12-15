#include "Renderer/pch.hpp"
#include "MeshRendererComponentBase.hpp"

#include "Renderer/RendererManager.hpp"

#include <ResourceManager/ResourceManager.hpp>

#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "MeshRendererComponentBase.sgenerated.hpp"

using namespace D_RENDERER;
using namespace D_RESOURCE;

namespace Darius::Renderer
{

	D_H_COMP_DEF(MeshRendererComponentBase);

	MeshRendererComponentBase::MeshRendererComponentBase() :
		RendererComponent(),
		mComponentPsoFlags(0),
		mLoD(1.f)
	{
		SetDirty();
	}

	MeshRendererComponentBase::MeshRendererComponentBase(D_CORE::Uuid uuid) :
		RendererComponent(uuid),
		mComponentPsoFlags(0),
		mLoD(1.f)
	{
		SetDirty();
	}

	void MeshRendererComponentBase::Awake()
	{
		// Initializing Mesh Constants buffers
		mMeshConstantsCPU.Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants), 3);
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

		for (UINT i = 0; i < mMaterials.size(); i++)
		{
			auto const& material = mMaterials[i];
			if (!material.IsValid())
				SetMaterial(i, static_cast<MaterialResource*>(D_RESOURCE::GetRawResourceSync(D_RENDERER::GetDefaultGraphicsResource(DefaultResource::Material))));
		}
	}

	void MeshRendererComponentBase::SetMaterial(UINT index, MaterialResource* material)
	{
		if (!CanChange())
			return;

		if (mMaterials[index] == material)
			return;
		
		mMaterialPsoData[index].PsoIndexDirty = true;

		mMaterials[index] = material;

		if (mMaterials[index].IsValid() && !material->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(material, nullptr, true);


		mChangeSignal(this);
	}

	UINT MeshRendererComponentBase::GetPsoIndex(UINT materialIndex)
	{
		auto materialPsoFlags = mMaterials[materialIndex]->GetPsoFlags();

		// Whether resource has changed
		if (mMaterialPsoData[materialIndex].CachedMaterialPsoFlags != materialPsoFlags)
		{
			mMaterialPsoData[materialIndex].CachedMaterialPsoFlags = materialPsoFlags;
			mMaterialPsoData[materialIndex].PsoIndexDirty = true;
		}

		// Whether pso index is not compatible with current pso flags
		if (mMaterialPsoData[materialIndex].PsoIndexDirty)
		{
			mMaterialPsoData[materialIndex].PsoIndex = D_RENDERER_RAST::GetPso({ (UINT16)(materialPsoFlags | mComponentPsoFlags) });
			mMaterialPsoData[materialIndex].DepthPsoIndex = D_RENDERER_RAST::GetPso({ (UINT16)(materialPsoFlags | mComponentPsoFlags | RenderItem::DepthOnly) });

			mMaterialPsoData[materialIndex].PsoIndexDirty = false;
		}
		return mMaterialPsoData[materialIndex].PsoIndex;
	}


#ifdef _D_EDITOR
	bool MeshRendererComponentBase::DrawDetails(float params[])
	{
		auto valueChanged = false;

		valueChanged |= Super::DrawDetails(params);

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		for (UINT i = 0; i < mMaterials.size(); i++)
		{
			auto setter = [&, i](MaterialResource* resource)
			{
				SetMaterial(i, resource);
			};

			auto name = std::string("Material ") + std::to_string(i + 1);
			D_H_DETAILS_DRAW_PROPERTY(name.c_str());
			D_H_RESOURCE_SELECTION_DRAW(MaterialResource, mMaterials[i], "Select Material", setter, std::to_string(i));
		}

		// LoD
		{
			D_H_DETAILS_DRAW_PROPERTY("LoD");
			float lod = GetLoD();
			if (ImGui::SliderFloat("##lod", &lod, 1.f, 64.f))
			{
				SetLoD(lod);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;

	}
#endif

	void MeshRendererComponentBase::OnDestroy()
	{
		mMeshConstantsCPU.Destroy();
		mMeshConstantsGPU.Destroy();
	}

	void MeshRendererComponentBase::OnDeserialized()
	{
		OnMeshChanged();
	}

	void MeshRendererComponentBase::OnMeshChanged()
	{
		auto numberOfSubmeshes = GetNumberOfSubmeshes();
		mMaterials.resize(numberOfSubmeshes);
		mMaterialPsoData.resize(numberOfSubmeshes);
	}

}
