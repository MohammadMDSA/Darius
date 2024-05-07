#include "pch.hpp"
#include "AudioSourceComponent.hpp"

#include "AudioResource.hpp"

#include <ResourceManager/ResourceManager.hpp>

#include <Audio.h>

#if _D_EDITOR
#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <Scene/Utils/GameObjectDragDropPayload.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>
#endif // _D_EDITOR


#include "AudioSourceComponent.sgenerated.hpp"

namespace Darius::Audio
{

	D_H_COMP_DEF(AudioSourceComponent);

	AudioSourceComponent::AudioSourceComponent() :
		Super(),
		mVolume(1.f),
		mPan(0.f),
		mPitch(0.f),
		mPlayOnStart(false),
		mSpatialize(true),
		mLoop(false),
		mMute(false)
	{ }

	AudioSourceComponent::AudioSourceComponent(D_CORE::Uuid const& uuid) :
		Super(uuid),
		mVolume(1.f),
		mPan(0.f),
		mPitch(0.f),
		mPlayOnStart(false),
		mSpatialize(true),
		mLoop(false),
		mMute(false)
	{ }

	void AudioSourceComponent::Start()
	{
		if(IsPlayOnStart() && mSoundInstance)
			mSoundInstance->Play(IsLoop());
	}

	void AudioSourceComponent::Stop(bool immediate)
	{
		if(mSoundInstance)
			mSoundInstance->Stop(immediate);
	}

	void AudioSourceComponent::Play()
	{
		if(mSoundInstance)
			mSoundInstance->Play(IsLoop());
	}

	void AudioSourceComponent::Pause()
	{
		if(mSoundInstance)
			mSoundInstance->Pause();
	}

	void AudioSourceComponent::Resume()
	{
		if(mSoundInstance)
			mSoundInstance->Resume();
	}

	void AudioSourceComponent::SetPan(float pan)
	{
		pan = D_MATH::Clamp(pan, -1.f, 1.f);

		if(mPan == pan)
			return;

		mPan = pan;
		if(mSoundInstance)
			mSoundInstance->SetPan(mPan);

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetVolume(float vol)
	{
		vol = D_MATH::Clamp(vol, 0.f, 1.f);

		if(mVolume == vol)
			return;

		mVolume = vol;

		if(mSoundInstance)
			mSoundInstance->SetVolume(vol);

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetPitch(float pitch)
	{
		pitch = D_MATH::Clamp(pitch, 0.f, 1.f);

		if(mPitch == pitch)
			return;

		mPitch = pitch;
		if(mSoundInstance)
			mSoundInstance->SetPitch(pitch);

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetAudioResource(AudioResource* audio)
	{
		if(mAudioResource == audio)
			return;

		mAudioResource = audio;

		if(mAudioResource.IsNull())
			mSoundInstance.reset();
		else
		{
			OnAudioResourceChanged();
		}

		mChangeSignal(this);
	}

	void AudioSourceComponent::OnAudioResourceChanged()
	{
		if(mAudioResource.IsNull())
			return;

		auto sf = mAudioResource->GetSoundEffect();
		D_ASSERT(sf);
		mSoundInstance = sf->CreateInstance();
		D_ASSERT(mSoundInstance.get());
		SetupInstance();
	}

	void AudioSourceComponent::OnDeserialized()
	{
		OnAudioResourceChanged();
	}

	void AudioSourceComponent::SetupInstance()
	{
		if(!mSoundInstance)
			return;

		mSoundInstance->SetPitch(GetPitch());
		mSoundInstance->SetPan(GetPan());
		mSoundInstance->SetVolume(GetVolume());
	}

	void AudioSourceComponent::OnDestroy()
	{
		mSoundInstance.reset();
	}

	void AudioSourceComponent::SetPlayOnStart(bool playOnStart)
	{
		if(mPlayOnStart == playOnStart)
			return;

		mPlayOnStart = playOnStart;

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetLoop(bool loop)
	{
		if(mLoop == loop)
			return;

		mLoop = loop;

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetMute(bool mute)
	{
		if(mMute == mute)
			return;

		mMute = mute;

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetSpatialize(bool spatialize)
	{
		if(mSpatialize == spatialize)
			return;

		mSpatialize = spatialize;

		mChangeSignal(this);
	}

#if _D_EDITOR
	bool AudioSourceComponent::DrawDetails(float _[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		// Clip
		{
			D_H_DETAILS_DRAW_PROPERTY("Audio Clip");
			D_H_RESOURCE_SELECTION_DRAW(AudioResource, mAudioResource, "Select Clip", SetAudioResource);
		}

		// Pan
		{
			D_H_DETAILS_DRAW_PROPERTY("Pan");
			float value = GetPan();
			if(ImGui::SliderFloat("##Pan", &value, -1.f, 1.f))
			{
				SetPan(value);
				valueChanged = true;
			}
		}

		// Volume
		{
			D_H_DETAILS_DRAW_PROPERTY("Volume");
			float value = GetVolume();
			if(ImGui::SliderFloat("##Volume", &value, 0.f, 1.f))
			{
				SetVolume(value);
				valueChanged = true;
			}
		}

		// Pitch
		{
			D_H_DETAILS_DRAW_PROPERTY("Pitch");
			float value = GetPitch();
			if(ImGui::SliderFloat("##Pitch", &value, -1.f, 1.f))
			{
				SetPitch(value);
				valueChanged = true;
			}
		}

		// Play on Start
		{
			D_H_DETAILS_DRAW_PROPERTY("Play on Start");
			auto value = IsPlayOnStart();
			if(ImGui::Checkbox("##PlayOnStart", &value))
			{
				SetPlayOnStart(value);
				valueChanged = true;
			}
		}

		// Loop
		{
			D_H_DETAILS_DRAW_PROPERTY("Loop");
			auto value = IsLoop();
			if(ImGui::Checkbox("##Loop", &value))
			{
				SetLoop(value);
				valueChanged = true;
			}
		}

		// Spatial
		{
			D_H_DETAILS_DRAW_PROPERTY("Spatialize");
			auto value = IsSpatialize();
			if(ImGui::Checkbox("##Spatialize", &value))
			{
				SetSpatialize(value);
				valueChanged = true;
			}
		}

		// Mute
		{
			D_H_DETAILS_DRAW_PROPERTY("Mute");
			auto value = IsMute();
			if(ImGui::Checkbox("##Mute", &value))
			{
				SetMute(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return valueChanged;

	}
#endif // _D_EDITOR

}