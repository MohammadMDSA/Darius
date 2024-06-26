#pragma once

#include "SceneProxy.hpp"

#include <Core/Signal.hpp>
#include <Math/Bounds/BoundingBox.hpp>
#include <Math/Bounds/DynamicBVH.hpp>

#include "SpacialSceneProxy.generated.hpp"

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Math
{
	struct Transform;
}

namespace Darius::Scene
{
	template <typename Object>
	class SpacialSceneProxy : public SceneProxy<Object>
	{
	public:
		SpacialSceneProxy(SceneManager* scene) :
			SceneProxy<Object>(scene)
		{ }

		virtual void						Update(float /*dt*/)
		{
			mBVH.OptimizeIncremental(1);
		}

		INLINE D_MATH_BOUNDS::DynamicBVH<Object*>& GetBVH() { return mBVH; }
		INLINE D_MATH_BOUNDS::DynamicBVH<Object*> const& GetBVH() const { return mBVH; }

	private:
		D_MATH_BOUNDS::DynamicBVH<Object*> mBVH;
	};

	template <typename Object>
	class SpacialSceneProxyObject : public SceneProxyObjectBase<Object>
	{
	public:
		SpacialSceneProxyObject(GameObject* go, SceneProxy<Object>* scene) :
			SceneProxyObjectBase<Object>(go, scene)
		{
			D_ASSERT(dynamic_cast<SpacialSceneProxy<Object>*>(scene));
			mTransformChangeSignal = go->GetTransform()->mWorldChanged.ConnectGenericObject(this, &SpacialSceneProxyObject::OnWorldTransformChanged);
		}

		virtual void Initialize() override
		{
			// Creating bvh node
			mBvhNodeId = GetSpacialScene()->GetBVH().Insert(GetAabb(), reinterpret_cast<Object*>(this));
		}

		virtual ~SpacialSceneProxyObject()
		{
			GetSpacialScene()->GetBVH().Remove(mBvhNodeId);
			mTransformChangeSignal.disconnect();
		}

		virtual void						OnWorldTransformChanged(D_MATH::TransformComponent* target, D_MATH::Transform const& newTrans)
		{
			GetSpacialScene()->GetBVH().Update(mBvhNodeId, GetAabb());
		}

		virtual D_MATH_BOUNDS::Aabb			GetAabb() const { return D_MATH_BOUNDS::Aabb(); };

		void								UpdateBvhNode()
		{
			GetSpacialScene()->GetBVH().Update(mBvhNodeId, GetAabb());
		}

		INLINE SpacialSceneProxy<Object>*	GetSpacialScene() const { return static_cast<SpacialSceneProxy<Object>*>(this->GetScene()); }

	private:
		
		D_CORE::SignalConnection			mTransformChangeSignal;
		D_MATH_BOUNDS::DynamicBVH<Object*>::ID mBvhNodeId;
	};
}
