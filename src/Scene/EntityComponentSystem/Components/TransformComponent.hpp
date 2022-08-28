#pragma once

#include "Scene/EntityComponentSystem/Components/ComponentBase.hpp"

#include <Core/Signal.hpp>
#include <Math/Transform.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

namespace Darius::Scene::ECS::Components
{
	class TransformComponent : public ComponentBase
	{
	public:
		INLINE D_MATH::Transform const*		GetData() const { return &mTransform; }
		INLINE D_MATH::Transform			GetData() { return mTransform; }
		INLINE void							SetTransform(D_MATH::Transform const& trans) { mTransform = trans; mChangeSignal(); }

		virtual bool						DrawDetails(float params[]) override;

		INLINE operator D_MATH::Transform const* () const { return &mTransform; }

		D_H_COMP_BODY(TransformComponent, ComponentBase);

	private:
		D_MATH::Transform					mTransform;
		D_CORE::Signal<void(void)>			mChangeSignal;
	};
}
