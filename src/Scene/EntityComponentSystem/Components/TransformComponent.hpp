#pragma once

#include "ComponentBase.hpp"

#include <Core/Signal.hpp>
#include <Math/Transform.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

namespace Darius::Scene::ECS::Components
{
	class TransformComponent : public ComponentBase
	{
		D_H_COMP_BODY(TransformComponent, ComponentBase);
	public:

		INLINE D_MATH::Transform const*		GetData() const { return &mTransform; }
		INLINE D_MATH::Transform			GetData() { return mTransform; }
		INLINE void							SetTransform(D_MATH::Transform const& trans) { mTransform = trans; mChangeSignal(); }

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		INLINE operator D_MATH::Transform const* () const { return &mTransform; }

	private:
		D_MATH::Transform					mTransform;
		D_CORE::Signal<void(void)>			mChangeSignal;
	};
}
