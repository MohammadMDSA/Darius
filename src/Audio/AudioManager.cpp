#include "pch.hpp"
#include "AudioManager.hpp"

#include "AudioResource.hpp"
#include "AudioSourceComponent.hpp"

#include <Core/Application.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Log.hpp>

#include <Audio.h>

bool											_initialize = false;
std::unique_ptr<DirectX::AudioEngine>			AudioEngineInst;
std::atomic_bool								ShouldReset = false;
D_CORE::SignalConnection						AppSuspendSignalConnection;
D_CORE::SignalConnection						AppResumeSignalConnection;
D_CORE::SignalConnection						NewDeviceSignalConnection;

using namespace DirectX;

namespace Darius::Audio
{
	void SuspendAudioEngine();
	void ResumeAudioEngine();
	void NewAudioDeviceConnected();

	void Initialize(D_SERIALIZATION::Json const&)
	{
		D_ASSERT(!_initialize);
		_initialize = true;
		ShouldReset = false;

		// Setting up audio engine
		AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
		eflags |= AudioEngine_Debug;
#endif

		auto enumList = AudioEngine::GetRendererDetails();

		DirectX::AudioEngine::RendererDetail det;
		if(enumList.empty())
		{
			// No audio devices
		}
		else
		{
			for(const auto& it : enumList)
			{
				det = it;
				int i = 1;
				if(i)
					continue;
			}
		}

		AudioEngineInst = std::make_unique<DirectX::AudioEngine>(eflags, nullptr, det.deviceId.c_str());

		AppSuspendSignalConnection = D_APP::SubscribeOnAppSuspended(SuspendAudioEngine);
		AppResumeSignalConnection = D_APP::SubscribeOnAppResuming(ResumeAudioEngine);
		NewDeviceSignalConnection = D_APP::SubscribeOnNewAudioDeviceConnected(NewAudioDeviceConnected);

		AudioResource::Register();
		AudioSourceComponent::StaticConstructor();
	}

	void Shutdown()
	{
		D_ASSERT(_initialize);

		AudioEngineInst->Suspend();
		AudioEngineInst.reset();

		AppResumeSignalConnection.disconnect();
		AppSuspendSignalConnection.disconnect();
		NewDeviceSignalConnection.disconnect();
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	void Update()
	{
		if(!AudioEngineInst->Update())
		{
			if(AudioEngineInst->IsCriticalError())
			{
				D_LOG_ERROR("Critical error detected in audio engine.");
			}
			ShouldReset.store(true);
		}

		if(ShouldReset.load())
		{
			ShouldReset.store(false);
			if(!AudioEngineInst->Reset())
			{
				D_LOG_ERROR("Could not reset the audio engine due to unavailability of any audio devices.");
			}
			else
			{
				D_LOG_INFO("Audio engine reset successfully; all sound instances have been stoped and need resetting.");
			}
		}
	}

	void SuspendAudioEngine()
	{
		AudioEngineInst->Suspend();
	}

	void ResumeAudioEngine()
	{
		AudioEngineInst->Resume();
	}

	void NewAudioDeviceConnected()
	{
		if(!AudioEngineInst)
			return;

		if(AudioEngineInst->IsAudioDevicePresent())
			return;

		ShouldReset.store(true);
	}

	DirectX::AudioEngine* GetEngineInstance()
	{
		return AudioEngineInst.get();
	}
}
