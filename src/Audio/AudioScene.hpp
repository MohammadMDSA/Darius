#pragma once

#include <Core/Containers/Vector.hpp>
#include <Scene/Proxy/SpacialSceneProxy.hpp>
#include <Scene/EntityComponentSystem/CompRef.hpp>

#include "AudioScene.generated.hpp"

#ifndef D_AUDIO
#define D_AUDIO Darius::Audio
#endif // !D_AUDIO

namespace Darius::Scene::ECS::Components
{
	class ComponentBase;
}

namespace Darius::Audio
{
	class AudioSourceComponent;
	class AudioScene;

	class AudioSceneObject : public D_SCENE::SpacialSceneProxyObject<AudioSceneObject>
	{
	public:
		AudioSceneObject(D_SCENE::GameObject* go, D_SCENE::SceneProxy<AudioSceneObject>* scene);
		~AudioSceneObject();

		virtual D_MATH_BOUNDS::Aabb				GetAabb() const override;

		INLINE D_ECS::CompRef<AudioSourceComponent> GetAudioSourceComoponent() const { return mRefComp; }

		void									OnAudioSourceChanged(Darius::Scene::ECS::Components::ComponentBase* src);

	private:

		D_ECS::CompRef<AudioSourceComponent>	mRefComp;
		D_CORE::SignalConnection				mChangeConnection;
		float									mRange;
	};

	class AudioScene : public D_SCENE::SpacialSceneProxy<AudioSceneObject>
	{
	public:
		AudioScene(D_SCENE::SceneManager* scene) :
			SpacialSceneProxy<AudioSceneObject>(scene)
		{}

		bool									RegisterAudioSource(AudioSourceComponent* src);
		bool									RemoveAudioSource(AudioSourceComponent* src);

		void									GetInRangeAudioSources(D_MATH::Vector3 const& position, float range, D_CONTAINERS::DVector<D_ECS::CompRef<AudioSourceComponent>>& inRangeSources) const;

	private:

	};
}
