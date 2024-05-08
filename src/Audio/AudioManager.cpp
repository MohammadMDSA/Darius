#include "pch.hpp"
#include "AudioManager.hpp"

#include "AudioResource.hpp"
#include "AudioSourceComponent.hpp"

#include <Core/Application.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Log.hpp>

#include <Audio.h>

#if _D_EDITOR
#include <imgui.h>
#endif // _EDITOR


bool											_initialize = false;
std::unique_ptr<DirectX::AudioEngine>			AudioEngineInst;
std::atomic_bool								ShouldReset = false;
D_CORE::SignalConnection						AppSuspendSignalConnection;
D_CORE::SignalConnection						AppResumeSignalConnection;
D_CORE::SignalConnection						NewDeviceSignalConnection;

// Options
std::string										PreferredDeviceId = "";

using namespace DirectX;

namespace Darius::Audio
{
	void SuspendAudioEngine();
	void ResumeAudioEngine();
	void NewAudioDeviceConnected();

	bool IsDevicePresent(std::wstring const& deviceId)
	{
		auto enumList = AudioEngine::GetRendererDetails();
		if(!enumList.empty())
		{
			for(const auto& it : enumList)
			{
				if(std::wstring(it.deviceId) == deviceId)
				{
					return true;
				}
			}
		}
		return false;
	}

	std::wstring GetAvailablePreferredDeviceIfAvailable()
	{
		std::wstring pref = STR2WSTR(PreferredDeviceId);
		return IsDevicePresent(pref) ? pref : std::wstring();
	}

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialize);
		_initialize = true;
		ShouldReset = false;

		D_H_OPTIONS_LOAD_BASIC_DEFAULT("Audio.PreferredDeviceId", PreferredDeviceId, "");

		// Setting up audio engine
		AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
		eflags |= AudioEngine_Debug;
#endif

		AudioEngineInst = std::make_unique<DirectX::AudioEngine>(eflags, nullptr, GetAvailablePreferredDeviceIfAvailable().c_str());

		AppSuspendSignalConnection = D_APP::SubscribeOnAppDeactivated(SuspendAudioEngine);
		AppResumeSignalConnection = D_APP::SubscribeOnAppActivated(ResumeAudioEngine);
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
		D_H_OPTION_DRAW_BEGIN();

		{
			D_CONTAINERS::DVector<char const*> deviceTextsC;
			D_CONTAINERS::DVector<std::string> deviceTexts;
			D_CONTAINERS::DVector<std::string> deviceIds;
			std::string selectedText = "";
			for(auto const& it : AudioEngine::GetRendererDetails())
			{
				std::string deviceText = WSTR2STR(it.description);
				deviceTexts.push_back(deviceText);
				deviceTextsC.push_back(deviceTexts.back().c_str());
				std::string deviceId = WSTR2STR(it.deviceId);
				deviceIds.push_back(deviceId);
				if(PreferredDeviceId == deviceId)
					selectedText = deviceText;
			}
			bool alreadyChanged = settingsChanged;
			D_H_OPTION_DRAW_CHOICE_VAL("Preferred Device", "Audio.PreferredDeviceId", PreferredDeviceId, deviceTextsC, deviceIds, selectedText.c_str(), deviceIds.size());
			if(!alreadyChanged && settingsChanged)
				ShouldReset.store(true);
		}

		D_H_OPTION_DRAW_END();

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
			if(!AudioEngineInst->Reset(nullptr, GetAvailablePreferredDeviceIfAvailable().c_str()))
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
