#pragma once

#include <Core/Serialization/Json.hpp>

#ifndef D_AUDIO
#define D_AUDIO Darius::Audio
#endif // !D_AUDIO

namespace DirectX
{
	class AudioEngine;
}

namespace Darius::Audio
{
	void					Initialize(D_SERIALIZATION::Json const& settings);
	void					Shutdown();
	void					Update();

#ifdef _D_EDITOR
	bool					OptionsDrawer(D_SERIALIZATION::Json& options);
#endif

	DirectX::AudioEngine*	GetEngineInstance();
}
