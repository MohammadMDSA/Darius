#pragma once

#include "MeshRendererComponentBase.hpp"

#include <Core/Ref.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>

#include "MeshRendererComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
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

		bool								AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction);

		INLINE virtual bool					CanRender() const override { return MeshRendererComponentBase::CanRender() && mMesh.IsValid(); }
		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() const override { return mMesh.Get()->GetMeshData()->mBoundSp; }

	private:

		DField(Resource, Serialize)
		D_CORE::Ref<StaticMeshResource>		mMesh;

	public:
		Darius_Graphics_MeshRendererComponent_GENERATED

	};
}

File_MeshRendererComponent_GENERATED