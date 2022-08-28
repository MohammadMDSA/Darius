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
		D_H_COMP_BODY(MeshRendererComponent, ComponentBase);
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


		INLINE bool							CanRender() { return mActive && GetGameObject()->GetActive() && mMeshResource.IsValid(); }
		INLINE const BoundingSphere&		GetBounds() const { return mMeshResource.Get()->GetData()->mBoundSp; }

		D_CH_RW_FIELD(bool, Active);

	private:

		void								Initialize();

		Ref<MeshResource>					mMeshResource;
		Ref<MaterialResource>				mMaterialResource;
	};
}