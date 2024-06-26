#include "pch.hpp"
#include "AudioSourceComponent.hpp"

#include "AudioResource.hpp"
#include "AudioManager.hpp"
#include "AudioScene.hpp"

#include <Physics/Components/RigidbodyComponent.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

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

	struct AudioSourceComponent::Impl
	{
		std::unique_ptr<DirectX::SoundEffectInstance> mSoundInstance = nullptr;
		std::unique_ptr<DirectX::AudioEmitter>	mEmitterData = nullptr;
	};

	D_MEMORY::PagedAllocator<AudioSourceComponent::Impl> AudioSourceComponent::DataAllocator;

	AudioSourceComponent::AudioSourceComponent() :
		Super(),
		mVolume(1.f),
		mPan(0.f),
		mPitch(0.f),
		mPlayOnStart(false),
		mSpatialize(true),
		mLoop(false),
		mMute(false),
		mDataImpl(nullptr),
		mMaxRange(100.f)
	{ }

	AudioSourceComponent::AudioSourceComponent(D_CORE::Uuid const& uuid) :
		Super(uuid),
		mVolume(1.f),
		mPan(0.f),
		mPitch(0.f),
		mPlayOnStart(false),
		mSpatialize(true),
		mLoop(false),
		mMute(false),
		mDataImpl(nullptr),
		mMaxRange(100.f)
	{ }

	void AudioSourceComponent::Awake()
	{
		if(!mDataImpl)
			mDataImpl = DataAllocator.Alloc();
	}

	void AudioSourceComponent::Start()
	{
		if(IsPlayOnStart())
			Play();

		D_AUDIO::GetAudioScene()->RegisterAudioSource(this);
	}

	void AudioSourceComponent::Stop(bool immediate)
	{
		if(mDataImpl->mSoundInstance)
			mDataImpl->mSoundInstance->Stop(immediate);
	}

	void AudioSourceComponent::Play()
	{
		if(mDataImpl->mSoundInstance)
		{
			if(IsSpatialize())
			{
				auto const* listener = D_AUDIO::GetListenerData();
				if(!listener)
					return;

				mDataImpl->mSoundInstance->Play(IsLoop());
				Apply3D(*listener, 0.0001f);
			}
			else
				mDataImpl->mSoundInstance->Play(IsLoop());
		}
	}

	void AudioSourceComponent::Pause()
	{
		if(mDataImpl->mSoundInstance)
			mDataImpl->mSoundInstance->Pause();
	}

	void AudioSourceComponent::Resume()
	{
		if(mDataImpl->mSoundInstance)
			mDataImpl->mSoundInstance->Resume();
	}

	void AudioSourceComponent::SetPan(float pan)
	{
		pan = D_MATH::Clamp(pan, -1.f, 1.f);

		if(mPan == pan)
			return;

		mPan = pan;
		if(mDataImpl->mSoundInstance)
			mDataImpl->mSoundInstance->SetPan(mPan);

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetVolume(float vol)
	{
		vol = D_MATH::Clamp(vol, 0.f, 1.f);

		if(mVolume == vol)
			return;

		mVolume = vol;

		if(mDataImpl->mSoundInstance)
			mDataImpl->mSoundInstance->SetVolume(vol);

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetPitch(float pitch)
	{
		pitch = D_MATH::Clamp(pitch, 0.f, 1.f);

		if(mPitch == pitch)
			return;

		mPitch = pitch;
		if(mDataImpl->mSoundInstance)
			mDataImpl->mSoundInstance->SetPitch(pitch);

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetAudioResource(AudioResource* audio)
	{
		if(mAudioResource == audio)
			return;

		mAudioResource = audio;

		if(mAudioResource.IsValid() && !mAudioResource->IsLoaded())
		{
			D_RESOURCE_LOADER::LoadResourceAsync(mAudioResource.Get(), [&](D_RESOURCE::Resource* loadedRes)
				{
					OnAudioResourceChanged();

				}, true);
		}
		else
			OnAudioResourceChanged();


		mChangeSignal(this);
	}

	void AudioSourceComponent::OnAudioResourceChanged()
	{
		if(mAudioResource.IsNull())
			mDataImpl->mSoundInstance.reset();
		else
			SetupInstance();
	}

	void AudioSourceComponent::OnPreDeserialize()
	{
		if(!mDataImpl)
			mDataImpl = DataAllocator.Alloc();
	}

	void AudioSourceComponent::OnDeserialized()
	{

		OnAudioResourceChanged();
	}

	void AudioSourceComponent::SetupInstance()
	{
		auto sf = mAudioResource->GetSoundEffect();
		D_ASSERT(sf);

		DirectX::SOUND_EFFECT_INSTANCE_FLAGS flags = IsSpatialize() ? DirectX::SoundEffectInstance_Use3D : DirectX::SoundEffectInstance_Default;

		mDataImpl->mSoundInstance = sf->CreateInstance(flags);
		if(!D_VERIFY(mDataImpl->mSoundInstance.get()))
			return;

		mDataImpl->mSoundInstance->SetPitch(GetPitch());
		mDataImpl->mSoundInstance->SetPan(GetPan());
		mDataImpl->mSoundInstance->SetVolume(GetVolume());
		SetupEmitter();
	}

	void AudioSourceComponent::OnDestroy()
	{
		D_AUDIO::GetAudioScene()->RemoveAudioSource(this);
		DataAllocator.Free(mDataImpl);
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
		SetupInstance();

		mChangeSignal(this);
	}

	void AudioSourceComponent::SetupEmitter()
	{
		if(!mSpatialize)
		{
			mDataImpl->mEmitterData.reset();
			return;
		}

		mDataImpl->mEmitterData = std::make_unique<DirectX::AudioEmitter>();
		mDataImpl->mEmitterData->SetOmnidirectional();
		mDataImpl->mEmitterData->pLPFDirectCurve = nullptr;
		mDataImpl->mEmitterData->pLPFReverbCurve = nullptr;
		mDataImpl->mEmitterData->pReverbCurve = nullptr;
		mDataImpl->mEmitterData->CurveDistanceScaler = GetMaxRange();

		static D_CONTAINERS::DVector<X3DAUDIO_DISTANCE_CURVE_POINT> curvePoints;
		static X3DAUDIO_DISTANCE_CURVE invCubicCurve;
		struct CurveInitializer
		{
			CurveInitializer()
			{
				const UINT NumPts = 6;
				curvePoints.resize(NumPts);

				for(int i = 0; i < NumPts; i++)
				{
					float dist = 1.f * i / (NumPts - 1);
					float intensity = 1.f - (dist * dist);
					curvePoints[i] = {dist, intensity};
				}
				invCubicCurve = {curvePoints.data(), NumPts};
			}
		};
		static CurveInitializer _initializer;
		(_initializer);

		mDataImpl->mEmitterData->pVolumeCurve = &invCubicCurve;
		mDataImpl->mEmitterData->pLFECurve = &invCubicCurve;
	}

	void AudioSourceComponent::SetMaxRange(float range)
	{
		range = D_MATH::Max(0.f, range);
		if(range == mMaxRange)
			return;

		mMaxRange = range;

		if(mDataImpl->mEmitterData)
			mDataImpl->mEmitterData->CurveDistanceScaler = GetMaxRange();

		mChangeSignal(this);
	}

	void AudioSourceComponent::Apply3D(X3DAUDIO_LISTENER const& listener, float dt)
	{
		using namespace DirectX;

		auto trans = GetTransform();

		mDataImpl->mEmitterData->SetOrientationFromQuaternion(trans->GetRotation());
		mDataImpl->mEmitterData->Update(trans->GetPosition(), trans->GetUp(), dt);

		if(mDataImpl->mSoundInstance)
			mDataImpl->mSoundInstance->Apply3D(listener, *mDataImpl->mEmitterData);
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

		// Max Range
		{
			D_H_DETAILS_DRAW_PROPERTY("Max Range");
			float value = GetMaxRange();
			if(ImGui::DragFloat("##MaxRange", &value, 0.5f, 0.f, FLT_MAX))
			{
				SetMaxRange(value);
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