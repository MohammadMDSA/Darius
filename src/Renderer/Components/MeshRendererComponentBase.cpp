#include "Renderer/pch.hpp"
#include "MeshRendererComponentBase.hpp"

#include <Renderer/FrameResource.hpp>

#include <ResourceManager/ResourceManager.hpp>

#include <Utils/DragDropPayload.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#endif

#include "MeshRendererComponentBase.sgenerated.hpp"

using namespace D_RENDERER_FRAME_RESOURCE;

namespace Darius::Graphics
{

	D_H_COMP_DEF(MeshRendererComponentBase);

	MeshRendererComponentBase::MeshRendererComponentBase() :
		D_ECS_COMP::ComponentBase(),
		mComponentPsoFlags(0),
		mCachedMaterialPsoFlags(0),
		mPsoIndex(0),
		mPsoIndexDirty(true)
	{
	}

	MeshRendererComponentBase::MeshRendererComponentBase(D_CORE::Uuid uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mComponentPsoFlags(0),
		mCachedMaterialPsoFlags(0),
		mPsoIndex(0),
		mPsoIndexDirty(true)
	{
	}

	void MeshRendererComponentBase::Awake()
	{
		// Initializing Mesh Constants buffers
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Create(L"Mesh Constant Upload Buffer", sizeof(MeshConstants));
		}
		mMeshConstantsGPU.Create(L"Mesh Constant GPU Buffer", 1, sizeof(MeshConstants));

		if (!mMaterial.IsValid())
			_SetMaterial(D_GRAPHICS::GetDefaultGraphicsResource(DefaultResource::Material));
	}

	uint16_t MeshRendererComponentBase::GetPsoIndex()
	{
		auto materialPsoFlags = mMaterial->GetPsoFlags();

		// Whether resource has changed
		if (mCachedMaterialPsoFlags != materialPsoFlags)
		{
			mCachedMaterialPsoFlags = materialPsoFlags;
			mPsoIndexDirty = true;
		}

		// Whether pso index is not compatible with current pso flags
		if (mPsoIndexDirty)
		{
			mPsoIndex = D_RENDERER::GetPso(materialPsoFlags | mComponentPsoFlags);
			mPsoIndexDirty = false;
		}
		return mPsoIndex;
	}


#ifdef _D_EDITOR
	bool MeshRendererComponentBase::DrawDetails(float params[])
	{
		auto valueChanged = false;

		// Material selection
		D_H_DETAILS_DRAW_PROPERTY("Material");
		D_H_RESOURCE_SELECTION_DRAW(MaterialResource, mMaterial, "Select Material", SetMaterial);

		// Casting shadow
		D_H_DETAILS_DRAW_PROPERTY("Casts Shadow");
		valueChanged |= ImGui::Checkbox("##CastsShadow", &mCastsShadow);

		return valueChanged;

	}
#endif

	void MeshRendererComponentBase::OnDestroy()
	{
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			mMeshConstantsCPU[i].Destroy();
		}
		mMeshConstantsGPU.Destroy();
	}
}
