#pragma once

#include "IRenderable.hpp"

#include "Renderer/Resources/MaterialResource.hpp"
#include "Renderer/Resources/StaticMeshResource.hpp"
#include "Renderer/Resources/TerrainResource.hpp"
#include "RendererComponent.hpp"

#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "TerrainRendererComponent.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) TerrainRendererComponent : public RendererComponent
	{
		GENERATED_BODY();
	public:
		enum class DEnum(Serialize) TerrainGridSize
		{
			Cells2x2,
			Cells4x4,
			Cells8x8,
			Cells16x16,
		};


		D_H_COMP_BODY(TerrainRendererComponent, RendererComponent, "Rendering/Terrain Renderer", true);

	public:

		virtual void						Update(float dt) override;
		virtual void						Awake() override;
		virtual void						OnDestroy() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) override;

		INLINE virtual D3D12_GPU_VIRTUAL_ADDRESS GetConstantsAddress() const override { return mMeshConstantsGPU.GetGpuVirtualAddress(); }
		virtual D_MATH_BOUNDS::Aabb			GetAabb() const override;

		INLINE virtual bool					IsDirty() const override { return D_ECS_COMP::ComponentBase::IsDirty() || GetTransform()->IsDirty(); }

		void								SetGridSize(TerrainGridSize size);

		void								SetTerrainData(TerrainResource* terrain);
		void								SetMaterial(MaterialResource* material);

		INLINE TerrainGridSize				GetGridSize() const { return mGridSize; }

		INLINE TerrainResource*				GetTerrainData() const { return mTerrainData.Get(); }
		INLINE MaterialResource*			GetMaterial() const { return mMaterial.Get(); }


		D_RESOURCE::ResourceRef<MaterialResource> GetMaterialRef() const { return mMaterial; }

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

		DField(Serialize)
		D_RESOURCE::ResourceRef<MaterialResource> mMaterial;

	private:
		
		DField(Serialize)
		TerrainGridSize						mGridSize;

		DField(Serialize)
		D_RESOURCE::ResourceRef<D_RENDERER::TerrainResource> mTerrainData;
		
		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer		mMeshConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;


		MaterialPsoData						mMaterialPsoData;
		D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource> mGridMesh;
	
	};
}

File_TerrainRendererComponent_GENERATED
