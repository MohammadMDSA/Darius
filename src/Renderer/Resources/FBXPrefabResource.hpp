#pragma once

#include <Scene/GameObject.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include <ResourceManager/ResourceRef.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include "FBXPrefabResource.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize, Resource) FBXPrefabResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(FBXPrefabResource, "FBX Scene", ".fbx");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

	protected:

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j) override;
		virtual bool					UploadToGpu() override;
		virtual INLINE void				Unload() override;

		INLINE virtual bool				AreDependenciesDirty() const override { return false; }


	protected:
		FBXPrefabResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false);

	private:

		DField(Serialize, Get)
		D_SCENE::GameObject* mPrefabGameObject;

	public:
		Darius_Renderer_FBXPrefabResource_GENERATED
	};
}

File_FBXPrefabResource_GENERATED
