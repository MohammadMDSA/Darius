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
		GENERATED_BODY();

		D_H_COMP_BODY(TransformComponent, D_ECS_COMP::ComponentBase, "Math/Transform", true);
	public:

		virtual INLINE void					SetEnabled(bool) override {}

		void								SetWorld(Matrix4 const& mat);
		void								SetLocalWorld(Matrix4 const& mat);

		void								SetLocalPosition(Vector3 const& value);
		void								SetLocalRotation(Quaternion const& val);
		void								SetLocalScale(Vector3 const& val);
		void								SetPosition(Vector3 const& val);
		void								SetRotation(Quaternion const& val);

		Matrix4 const&						GetWorld();
		INLINE Vector3 const&				GetLocalPosition() const { return mTransformMath.Translation; }
		INLINE Quaternion const&			GetLocalRotation() const { return mTransformMath.Rotation; }
		INLINE Vector3 const&				GetLocalScale() const { return mTransformMath.Scale; }
		Vector3								GetPosition();
		Quaternion							GetRotation();
		Vector3								GetScale();
		Vector3								GetForward();
		INLINE Transform const&				GetTransformData() const { return mTransformMath; }

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		INLINE virtual bool					IsDisableable() const override { return false; }

		INLINE virtual bool					IsDirty() const override { auto parent = GetGameObject()->GetParent(); return parent ? ComponentBase::IsDirty() || parent->GetTransform()->IsDirty() : ComponentBase::IsDirty(); }

	private:
		bool								IsWorldDirty() const;

		D_MATH::Transform					mTransformMath;
		D_MATH::Matrix4						mWorldMatrix;

		bool								mWorldDirty;
	};

	INLINE bool TransformComponent::IsWorldDirty() const
	{
		if (mWorldDirty)
			return true;

		auto parent = GetGameObject()->GetParent();

		if (parent != nullptr && parent->GetTransform()->IsDirty())
			return true;
		return false;
	}

	INLINE Matrix4 const& TransformComponent::GetWorld()
	{
		// Should update world first
		if (IsWorldDirty())
		{
			auto parent = GetGameObject()->GetParent();

			mWorldMatrix = Matrix4(mTransformMath.GetWorld());

			// If has parent, should apply their world too
			if (parent != nullptr)
				mWorldMatrix = parent->GetTransform()->GetWorld() * mWorldMatrix;

			mWorldDirty = false;
		}

		return mWorldMatrix;
	}

	INLINE void TransformComponent::SetLocalPosition(Vector3 const& value)
	{
		if (!CanChange())
			return;
		mTransformMath.Translation = value;
		mWorldDirty = true;
		SetDirty();
	}

	INLINE void TransformComponent::SetLocalRotation(Quaternion const& val)
	{
		if (!CanChange())
			return;
		mTransformMath.Rotation = val;
		mWorldDirty = true;
		SetDirty();
	}

	INLINE void TransformComponent::SetLocalScale(Vector3 const& val)
	{
		if (!CanChange())
			return;
		mTransformMath.Scale = val;
		mWorldDirty = true;
		SetDirty();
	}

	INLINE void TransformComponent::SetPosition(Vector3 const& val)
	{
		if (!CanChange())
			return;
		auto world = Transform(GetWorld());
		world.Translation = val;
		SetWorld(Matrix4(world.GetWorld()));
		SetDirty();
	}

	INLINE void TransformComponent::SetRotation(Quaternion const& val)
	{
		if (!CanChange())
			return;
		auto world = Transform(GetWorld());
		world.Rotation = val;
		SetWorld(Matrix4(world.GetWorld()));
		SetDirty();
	}

	INLINE Vector3 TransformComponent::GetPosition()
	{
		auto world = Transform(GetWorld());
		return world.Translation;
	}

	INLINE Quaternion TransformComponent::GetRotation()
	{
		auto world = Transform(GetWorld());
		return world.Rotation;
	}

	INLINE Vector3 TransformComponent::GetScale()
	{
		auto world = Transform(GetWorld());
		return world.Scale;
	}

	INLINE Vector3 TransformComponent::GetForward()
	{
		return GetRotation().GetForward();
	}
}

File_TransformComponent_GENERATED
