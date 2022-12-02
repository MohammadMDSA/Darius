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
		D_H_COMP_BODY(TransformComponent, ComponentBase, "Math/Transform", true, false);
	public:

		// Serialization
		virtual void						Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void						Deserialize(D_SERIALIZATION::Json const& j) override;

		virtual INLINE  void				SetEnabled(bool) override {}

		INLINE D_MATH::Transform const*		GetDataC() const { return &mTransform; }
		INLINE D_MATH::Transform			GetData() const { return mTransform; }
		INLINE void							SetLocalTransform(D_MATH::Transform const& trans) { mTransform = trans; mChangeSignal(); }

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		INLINE virtual bool					IsDisableable() const { return false; }

		INLINE operator D_MATH::Transform const* () const { return &mTransform; }

	private:
		D_MATH::Transform					mTransform;
	};
}
