#include "pch.hpp"
#include "AudioListenerComponent.hpp"

#include "AudioListenerComponent.sgenerated.hpp"

namespace Darius::Audio
{
	D_H_COMP_DEF(AudioListenerComponent);

	AudioListenerComponent::AudioListenerComponent() :
		Super()
	{ }

	AudioListenerComponent::AudioListenerComponent(D_CORE::Uuid const& uuid) :
		Super(uuid)
	{ }

}
