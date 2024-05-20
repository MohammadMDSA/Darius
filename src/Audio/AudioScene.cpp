#include "pch.hpp"
#include "AudioScene.hpp"

#include "AudioSourceComponent.hpp"

#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "AudioScene.sgenerated.hpp"

namespace Darius::Audio
{
	AudioSceneObject::AudioSceneObject(D_SCENE::GameObject* go, D_SCENE::SceneProxy<AudioSceneObject>* scene) :
		D_SCENE::SpacialSceneProxyObject<AudioSceneObject>(go, scene)
	{
		mRefComp = D_ECS::CompRef<AudioSourceComponent>(go);
		D_ASSERT(mRefComp.IsValid());

		mRefComp->mChangeSignal.ConnectGenericObject(this, &AudioSceneObject::OnAudioSourceChanged);
	}

	AudioSceneObject::~AudioSceneObject()
	{
		mChangeConnection.disconnect();
	}

	void AudioSceneObject::OnAudioSourceChanged(D_ECS_COMP::ComponentBase* src)
	{
		mAabb = D_MATH_BOUNDS::Aabb::CreateFromCenterAndExtents(GetGameObject()->GetTransform()->GetPosition(), D_MATH::Vector3(mRefComp->GetMaxRange()));

		UpdateBvhNode();
	}

	D_MATH_BOUNDS::Aabb AudioSceneObject::GetAabb() const
	{
		return mAabb;
	}

	bool AudioScene::RegisterAudioSource(AudioSourceComponent* src)
	{
		auto go = src->GetGameObject();
		if(!go || !go->IsValid())
			return false;

		return AddActor(go) != nullptr;
	}

	bool AudioScene::RemoveAudioSource(AudioSourceComponent* src)
	{
		return RemoveActor(src->GetGameObject());
	}

	void AudioScene::GetInRangeAudioSources(D_MATH::Vector3 const& position, float range, D_CONTAINERS::DVector<D_ECS::CompRef<AudioSourceComponent>>& inRangeSources) const
	{
		inRangeSources.clear();

		GetBVH().SphereQuery(D_MATH_BOUNDS::BoundingSphere(position, range), [&inRangeSources](AudioSceneObject* const& sceneObj, D_MATH_BOUNDS::Aabb const&)
			{
				inRangeSources.push_back(sceneObj->GetAudioSourceComoponent());
				return true;
			});
	}
}
