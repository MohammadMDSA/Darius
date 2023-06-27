#pragma once

#include "IRenderable.hpp"

#include "Renderer/Resources/MaterialResource.hpp"
#include "Renderer/Resources/StaticMeshResource.hpp"
#include "Renderer/Resources/TerrainResource.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "TerrainRendererComponent.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) TerrainRendererComponent : public D_ECS_COMP::ComponentBase, public IRenderable
	{
	public:
		enum class DEnum(Serialize) TerrainGridSize
		{
			Cells2x2,
			Cells4x4,
			Cells8x8,
			Cells16x16,
		};


		D_H_COMP_BODY(TerrainRendererComponent, D_ECS_COMP::ComponentBase, "Rendering/Terrain Renderer", true);

	public:

		virtual void						Update(float dt) override;
		virtual void						Awake() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float[]) override;
#endif // _D_EDITOR


		virtual bool						AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction);

		INLINE virtual bool					CanRender() const { return IsActive(); }
		INLINE virtual D3D12_GPU_VIRTUAL_ADDRESS GetConstantsAddress() const override { return mMeshConstantsGPU.GetGpuVirtualAddress(); }
		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() override { return mGridMesh->GetMeshData()->mBoundSp; }

		INLINE virtual bool					IsDirty() const override { return D_ECS_COMP::ComponentBase::IsDirty() || GetTransform()->IsDirty(); }

		void								SetGridSize(TerrainGridSize size);

		static INLINE std::string			GetTerrainSizeName(TerrainGridSize size)
		{
			switch (size)
			{
			case D_RENDERER::TerrainRendererComponent::TerrainGridSize::Cells2x2:
				return "Cells 2x2";
			case D_RENDERER::TerrainRendererComponent::TerrainGridSize::Cells4x4:
				return "Cells 4x4";
			case D_RENDERER::TerrainRendererComponent::TerrainGridSize::Cells8x8:
				return "Cells 8x8";
			case D_RENDERER::TerrainRendererComponent::TerrainGridSize::Cells16x16:
				return "Cells 16x16";
			default:
				D_ASSERT_M(false, "Bad grid size");
				return "";
			}
		}

	protected:
		void								UpdatePsoIndex();
		void								UpdateGridMesh();

		DField(Serialize, Resource)
		D_RESOURCE::ResourceRef<MaterialResource> mMaterial;

	private:
		
		DField(Get[inline], Set[inline])
		bool								mCastsShadow;

		DField(Get[inline], Serialize)
		TerrainGridSize						mGridSize;

		DField(Serialize, Resource)
		D_RESOURCE::ResourceRef<D_RENDERER::TerrainResource> mTerrainData;
		
		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer		mMeshConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;


		MaterialPsoData						mMaterialPsoData;
		D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource> mGridMesh;
	

	public:
		Darius_Renderer_TerrainRendererComponent_GENERATED
	};
}

File_TerrainRendererComponent_GENERATED
