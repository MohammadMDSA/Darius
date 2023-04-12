#pragma once

#include "ComponentBase.hpp"

#include <Core/Signal.hpp>
#include <Math/Transform.hpp>

#include "TransformComponent.generated.hpp"

#ifndef D_MATH
#define D_MATH Darius::Math
#endif // !D_MATH

namespace Darius::Math
{
	class DClass(Serialize[LocalPosition, LocalRotation, LocalScale]) TransformComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(TransformComponent, D_ECS_COMP::ComponentBase, "Math/Transform", true);
	public:

		// Serialization
		virtual void						Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void						Deserialize(D_SERIALIZATION::Json const& j) override;

		virtual INLINE void					SetEnabled(bool) override {}

		INLINE D_MATH::Transform const* GetDataC() const { return &mTransform; }
		INLINE D_MATH::Transform			GetData() const { return mTransform; }
		INLINE void							SetLocalTransform(D_MATH::Transform const& trans) { mTransform = trans; mChangeSignal(); }

		INLINE void							SetLocalPosition(Vector3 const& value) { mTransform.Translation = value; }
		INLINE void							SetLocalRotation(Quaternion const& val) { mTransform.Rotation = val; }
		INLINE void							SetLocalScale(Vector3 const& val) { mTransform.Scale = val; }

		INLINE Vector3 const&				GetLocalPosition() const { return mTransform.Translation; }
		INLINE Quaternion const&			GetLocalRotation() const { return mTransform.Rotation; }
		INLINE Vector3 const&				GetLocalScale() const { return mTransform.Scale; }

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		INLINE virtual bool					IsDisableable() const { return false; }

		INLINE operator D_MATH::Transform const* () const { return &mTransform; }

	private:
		D_MATH::Transform					mTransform;

	public:
		Darius_Math_TransformComponent_GENERATED
	};
}

File_TransformComponent_GENERATED
