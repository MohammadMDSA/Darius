#pragma once

#include "MeshRendererComponentBase.hpp"

#include "Renderer/Resources/StaticMeshResource.hpp"

#include <Core/RefCounting/Ref.hpp>

#include "MeshRendererComponent.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) MeshRendererComponent : public MeshRendererComponentBase
	{
		GENERATED_BODY();
		D_H_COMP_BODY(MeshRendererComponent, MeshRendererComponentBase, "Rendering/Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// States
		virtual void						Update(float dt) override;

		virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) override;
#if _D_EDITOR
		virtual D_RENDERER::RenderItem		GetPickerRenderItem() const override;
		virtual bool						CanRenderForPicker() const override;
#endif

		INLINE virtual UINT					GetNumberOfSubmeshes() const { return mMesh.IsValid() ? (UINT)mMesh->GetMeshData()->mDraw.size() : 0u; }
		INLINE virtual bool					CanRender() const override { return mMesh.IsValid() && MeshRendererComponentBase::CanRender(); }
		virtual D_MATH_BOUNDS::Aabb			GetAabb() const override;

		void								SetMesh(StaticMeshResource* mesh);
		INLINE StaticMeshResource*			GetMesh() const { return mMesh.Get(); }
		virtual void						GetOverriddenMaterials(D_CONTAINERS::DVector<MaterialResource*>& out) const override;

	protected:
		virtual UINT						GetPsoIndex(UINT materialIndex, MaterialResource* material) override;

	private:

		DField(Serialize)
		D_RESOURCE::ResourceRef<StaticMeshResource> mMesh;


	};
}

File_MeshRendererComponent_GENERATED