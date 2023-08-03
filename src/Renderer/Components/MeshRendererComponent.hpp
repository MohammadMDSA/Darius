#pragma once

#include "MeshRendererComponentBase.hpp"

#include "Renderer/Resources/StaticMeshResource.hpp"

#include <Core/Ref.hpp>

#include "MeshRendererComponent.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) MeshRendererComponent : public MeshRendererComponentBase
	{
		D_H_COMP_BODY(MeshRendererComponent, MeshRendererComponentBase, "Rendering/Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// States
		virtual void						Update(float dt) override;

		virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction) override;

		INLINE virtual UINT					GetNumberOfSubmeshes() const { return mMesh.IsValid() ? (UINT)mMesh->GetMeshData()->mDraw.size() : 0u; }
		INLINE virtual bool					CanRender() const override { return MeshRendererComponentBase::CanRender() && mMesh.IsValid(); }
		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() override { return mMesh.Get()->GetMeshData()->mBoundSp; }

	protected:
		virtual UINT						GetPsoIndex(UINT materialIndex) override;

	private:

		void								_SetMesh(D_RESOURCE::ResourceHandle handle);

		DField(Resource[false], Serialize)
		D_RESOURCE::ResourceRef<StaticMeshResource> mMesh;

	public:
		Darius_Renderer_MeshRendererComponent_GENERATED

	};
}

File_MeshRendererComponent_GENERATED