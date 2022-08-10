#pragma once

#include "Resource.hpp"

#include <Renderer/FrameResource.hpp>


#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

using namespace D_RENDERER_FRAME_RESOUCE;

namespace Darius::ResourceManager
{
	class DResourceManager;

	class MaterialResource : public Resource
	{
		D_CH_RESOUCE_BODY(MaterialResource, ResourceType::Material)
		
	public:
		INLINE Material*				Get() { mDirty = true; return &mMaterial; }
		INLINE const Material*			Get() const { return &mMaterial; }

		INLINE virtual ResourceType		GetType() const override { return ResourceType::Material; }

		virtual bool					Save() override;
		virtual bool					Load() override;
		virtual bool					SuppoertsExtension(std::wstring ext) override;

		INLINE operator const Material* const() { return &mMaterial; }
		INLINE operator Material* const() { return &mMaterial; }

		D_CH_FIELD(Material, Material);
	private:
		friend class DResourceManager;

		MaterialResource(std::wstring path, DResourceId id, bool isDefault = false) :
			Resource(path, id, isDefault) {}

	};
}