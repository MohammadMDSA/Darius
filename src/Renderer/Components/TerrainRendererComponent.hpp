#pragma once

#include "IRenderable.hpp"

#include "Renderer/Resources/MaterialResource.hpp"
#include "Renderer/Resources/StaticMeshResource.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "TerrainRendererComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS

namespace Darius::Graphics
{
	class DClass(Serialize) TerrainRendererComponent : public D_ECS_COMP::ComponentBase, public IRenderable
	{
	public:
		enum class DEnum(Serialize) TerrainGridSize
		{
			Cells1x1 = 0,
			Cells2x2,
			Cells4x4,
			Cells8x8,
			Cells16x16,
			Cells100x100
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

		void								SetGridSize(TerrainGridSize size);

		static INLINE std::string			GetTerrainSizeName(TerrainGridSize size)
		{
			switch (size)
			{
			case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells1x1:
				return "Cells 1x1";
			case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells2x2:
				return "Cells 2x2";
			case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells4x4:
				return "Cells 4x4";
			case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells8x8:
				return "Cells 8x8";
			case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells16x16:
				return "Cells 16x16";
			case Darius::Graphics::TerrainRendererComponent::TerrainGridSize::Cells100x100:
				return "Cells 100x100";
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
		D_RESOURCE::ResourceRef<D_GRAPHICS::TextureResource> mHeightMap;
		
		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer		mMeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;


		MaterialPsoData						mMaterialPsoData;
		D_RESOURCE::ResourceRef<D_GRAPHICS::StaticMeshResource> mGridMesh;
	

	public:
		Darius_Graphics_TerrainRendererComponent_GENERATED
	};
}

File_TerrainRendererComponent_GENERATED