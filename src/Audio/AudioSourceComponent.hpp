#pragma once

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include "AudioSourceComponent.generated.hpp"

#ifndef D_AUDIO
#define D_AUDIO Darius::Audio
#endif // !D_AUDIO

namespace DirectX
{
	class SoundEffectInstance;
}

namespace Darius::Audio
{
	class AudioResource;

	class DClass(Serialize[bPlayOnStart, bLoop, bSpatialize, bMute, Pan, Volume, Pitch]) AudioSourceComponent : public D_ECS_COMP::ComponentBase
	{
		GENERATED_BODY();
		D_H_COMP_BODY(AudioSourceComponent, D_ECS_COMP::ComponentBase, "Audio/Audio Source", true);

	public:

		virtual void							Start() override;
		virtual	void							Update(float dt) override { }
		virtual void							OnDestroy() override;
		virtual void							OnDeserialized() override;

#if _D_EDITOR
		virtual bool							DrawDetails(float params[]) override;
#endif

		// Audio Resource
		INLINE AudioResource*					GetAudioResource() const { return mAudioResource.Get(); }
		void									SetAudioResource(AudioResource* audio);

		// Play on Start
		INLINE bool								IsPlayOnStart() const { return mPlayOnStart; }
		void									SetPlayOnStart(bool playOnStart);

		// Loop
		INLINE bool								IsLoop() const { return mLoop; }
		void									SetLoop(bool loop);

		// Spatialize
		INLINE bool								IsSpatialize() const { return mSpatialize; }
		void									SetSpatialize(bool spatialize);

		// Mute
		INLINE bool								IsMute() const { return mMute; }
		void									SetMute(bool mute);

		// Pan
		INLINE float							GetPan() const { return mPan; }
		// -1 is fully left and +1 is fully right, and 0 is balanced.
		void									SetPan(float);

		// Volume
		INLINE float							GetVolume() const { return mVolume; }
		void									SetVolume(float vol);

		// Pitch
		INLINE float							GetPitch() const { return mPitch; }
		void									SetPitch(float pitch);

		// If is looped, plays until the end of the clip. If Immediate is set, stops immediately
		void									Stop(bool immediate);
		void									Play();
		void									Pause();
		void									Resume();

	protected:

		void									SetupInstance();
		void									OnAudioResourceChanged();

	private:

		DField(Serialize)
		D_RESOURCE::ResourceRef<AudioResource>	mAudioResource;

		std::unique_ptr<DirectX::SoundEffectInstance> mSoundInstance;

		float									mPan;
		float									mVolume;
		float									mPitch;

		uint8_t									mPlayOnStart : 1;
		uint8_t									mLoop : 1;
		uint8_t									mSpatialize : 1;
		uint8_t									mMute : 1;
	};
}

File_AudioSourceComponent_GENERATED
