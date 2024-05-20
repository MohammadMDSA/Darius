#pragma once

#include <Core/Serialization/Json.hpp>

#ifndef D_AUDIO
#define D_AUDIO Darius::Audio
#endif // !D_AUDIO

struct X3DAUDIO_LISTENER;

namespace DirectX
{
	class AudioEngine;
}

namespace Darius::Audio
{
	class AudioScene;

	void					Initialize(D_SERIALIZATION::Json const& settings);
	void					Shutdown();
	void					Update(float dt);

	X3DAUDIO_LISTENER const& GetListenerData();

#ifdef _D_EDITOR
	bool					OptionsDrawer(D_SERIALIZATION::Json& options);
#endif

	DirectX::AudioEngine*	GetEngineInstance();

	AudioScene*				GetAudioScene();
}
