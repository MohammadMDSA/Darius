#pragma once

#include "ComponentBase.hpp"

#include <Core/Ref.hpp>
#include <ResourceManager/ResourceTypes/StaticMeshResource.hpp>
#include <ResourceManager/ResourceTypes/MaterialResource.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

using namespace D_CORE;
using namespace D_RESOURCE;

namespace Darius::Scene::ECS::Components
{
	class MeshRendererComponent : public ComponentBase
	{
		D_H_COMP_BODY(MeshRendererComponent, ComponentBase, "Rendering/Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// Serialization
		virtual void						Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void						Deserialize(D_SERIALIZATION::Json const& j) override;

		// States
		virtual void						Start() override;
		virtual void						Update(float dt) override;
		virtual void						OnDestroy() override;

		void								SetMesh(ResourceHandle handle);
		void								SetMaterial(ResourceHandle handle);

		RenderItem							GetRenderItem();


		INLINE bool							CanRender() { return IsActive() && mMeshResource.IsValid(); }
		INLINE const BoundingSphere&		GetBounds() const { return mMeshResource.Get()->GetMeshData()->mBoundSp; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() { return mMeshConstantsGPU.GetGpuVirtualAddress(); }

	private:

		void								_SetMesh(ResourceHandle handle);
		void								_SetMaterial(ResourceHandle handle);

		Ref<StaticMeshResource>				mMeshResource;
		Ref<MaterialResource>				mMaterialResource;
		uint16_t							mPsoFlags;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMeshConstantsGPU;

	};
}