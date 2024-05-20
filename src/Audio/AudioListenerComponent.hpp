#pragma once

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include "AudioListenerComponent.generated.hpp"

#ifndef D_AUDIO
#define D_AUDIO Darius::Audio
#endif // !D_AUDIO

namespace DirectX
{
}

namespace Darius::Audio
{
	class AudioResource;

	class DClass(Serialize) AudioListenerComponent : public D_ECS_COMP::ComponentBase
	{
		GENERATED_BODY();
		D_H_COMP_BODY(AudioListenerComponent, D_ECS_COMP::ComponentBase, "Audio/Audio Listener", true);

	public:


	};
}

File_AudioListenerComponent_GENERATED
