#pragma once

#include <Scene/GameObject.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include <ResourceManager/ResourceRef.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include "FBXPrefabResource.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
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

	protected:
		FBXPrefabResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, bool isDefault = false);

	private:

		DField(Serialize, Get)
		D_SCENE::GameObject* mPrefabGameObject;

	public:
		Darius_Graphics_FBXPrefabResource_GENERATED
	};
}

File_FBXPrefabResource_GENERATED
