#pragma once

#include "Scene/GameObject.hpp"
#include "Scene/EntityComponentSystem/Components/TransformComponent.hpp"

#include <ResourceManager/ResourceManager.hpp>

#if _D_EDITOR
#include <ResourceManager/ResourceDragDropPayload.hpp>
#endif // _D_EDITOR


#include "PrefabResource.generated.hpp"

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	class DClass(Serialize, Resource) PrefabResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();

		D_CH_RESOURCE_BODY(PrefabResource, "Prefab", ".prefab");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		void							CreateFromGameObject(GameObject const* go);

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual INLINE bool				UploadToGpu() override { return true; }
		virtual void					Unload() override;

		INLINE virtual bool				AreDependenciesDirty() const override { return false; }

		INLINE GameObject*				GetPrefabGameObject() const { return mPrefabGameObject; }
		void							SetPrefabGameObject(D_SCENE::GameObject* go);

	protected:
		PrefabResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false);

	private:

		DField(Serialize)
		GameObject* mPrefabGameObject;

	};

#if _D_EDITOR

	struct PrefabResourceDragDropPayloadContent : public D_RESOURCE::ResourceDragDropPayloadContent
	{
		virtual inline bool IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type payloadType, D_CORE::StringId const& type) const override
		{
			if(D_RESOURCE::ResourceDragDropPayloadContent::IsCompatible(payloadType, type))
				return true;

			if(payloadType == D_UTILS::BaseDragDropPayloadContent::Type::GameObject)
				return true;

			return false;

		}

		virtual Darius::Scene::GameObject* GetAssociatedGameObject() const override;
	};
#endif // _D_EDITOR

}

File_PrefabResource_GENERATED
