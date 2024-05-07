#pragma once

#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include "AudioSourceResource.generated.hpp"

#ifndef D_AUDIO
#define D_AUDIO Darius::Audio
#endif // !D_AUDIO

namespace DirectX
{
	class SoundEffect;
}

namespace Darius::Audio
{
	class DClass(Serialize, Resource) AudioSourceResource : public D_RESOURCE::Resource
	{
		GENERATED_BODY();

		D_CH_RESOURCE_BODY(AudioSourceResource, "Audio Source", ".wav");

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		~AudioSourceResource();

	protected:

		virtual void					WriteResourceToFile(D_SERIALIZATION::Json& j) const override;
		virtual void					ReadResourceFromFile(D_SERIALIZATION::Json const& j, bool& dirtyDisk) override;
		virtual bool					UploadToGpu() override;
		virtual INLINE void				Unload() override { EvictFromGpu(); }
		virtual INLINE bool				AreDependenciesDirty() const { return false; }


	private:
		AudioSourceResource(D_CORE::Uuid uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault = false) :
			D_RESOURCE::Resource(uuid, path, name, id, parent, isDefault),
			mSoundEffectData(nullptr)
		{}


		DirectX::SoundEffect*			mSoundEffectData;
	};
}

File_AudioSourceResource_GENERATED
