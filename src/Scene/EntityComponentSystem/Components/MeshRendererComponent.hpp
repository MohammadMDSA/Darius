#pragma once

#include "ComponentBase.hpp"

#include <Core/Ref.hpp>
#include <ResourceManager/ResourceTypes/MeshResource.hpp>
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
		D_H_COMP_BODY(MeshRendererComponent, ComponentBase, "Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// Serialization
		virtual void						Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void						Deserialize(D_SERIALIZATION::Json const& j) override;

		// States
		virtual void						Start() override;

		void								SetMesh(ResourceHandle handle);
		void								SetMaterial(ResourceHandle handle);

		RenderItem							GetRenderItem();


		INLINE bool							CanRender() { return GetGameObject()->GetActive() && GetEnabled() && mMeshResource.IsValid(); }
		INLINE const BoundingSphere&		GetBounds() const { return mMeshResource.Get()->GetMeshData()->mBoundSp; }

	private:

		void								_SetMesh(ResourceHandle handle);
		void								_SetMaterial(ResourceHandle handle);

		Ref<MeshResource>					mMeshResource;
		Ref<MaterialResource>				mMaterialResource;
		D_CORE::Signal<void()>				mChangeSignal;
	};
}