#pragma once

#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceTypes/Resource.hpp>

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

using namespace D_RESOURCE;

namespace Darius::Animation
{
	class AnimationResource : public D_RESOURCE::Resource
	{
		D_CH_RESOURCE_BODY(AnimationResource, "Animation", ".fbx");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override { return false; }
#endif // _D_EDITOR

		virtual INLINE void				WriteResourceToFile() const {};
		virtual INLINE void				ReadResourceFromFile() {};
		virtual bool					UploadToGpu(D_GRAPHICS::GraphicsContext& context) override;

		static DVector<ResourceDataInFile> CanConstructFrom(ResourceType type, Path const& path);

	protected:
		AnimationResource(Uuid uuid, std::wstring const& path, std::wstring const& name, DResourceId id, bool isDefault = false) :
			Resource(uuid, path, name, id, isDefault) {}
	};
}
