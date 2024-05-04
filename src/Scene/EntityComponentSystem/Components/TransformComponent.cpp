#include "Scene/pch.hpp"
#include "TransformComponent.hpp"

#include "Scene/Utils/DetailsDrawer.hpp"

#include <Math/Serialization.hpp>

#include "TransformComponent.sgenerated.hpp"

using namespace D_SERIALIZATION;

namespace Darius::Math
{
	D_H_COMP_DEF(TransformComponent);

	TransformComponent::TransformComponent() :
		D_ECS_COMP::ComponentBase(),
		mTransformMath(Vector3::Zero, Quaternion::Identity, Vector3::One),
		mWorldMatrix(kZero),
		mWorldDirty(true),
		mWorldChanged()
	{
		SetDirty();
	}

	TransformComponent::TransformComponent(D_CORE::Uuid const& uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mTransformMath(Vector3::Zero, Quaternion::Identity, Vector3::One),
		mWorldMatrix(kZero),
		mWorldDirty(true),
		mWorldChanged()
	{
		SetDirty();
	}

	void TransformComponent::SetWorld(Matrix4 const& mat)
	{
		if (!CanChange())
			return;

		mWorldMatrix = mat;
		mWorldDirty = false;
		SetDirty();

		auto parent = GetGameObject()->GetParent();
		if (parent == nullptr)
			mTransformMath = Transform(mat);
		else
		{
			auto localWorld = D_MATH::Invert(parent->GetTransform()->GetWorld()) * mat;
			mTransformMath = Transform(localWorld);

		}
		mWorldChanged(this, mTransformMath);
	}

	Matrix4 const& TransformComponent::GetWorld()
	{
		// Should update world first
		if(IsWorldDirty())
		{
			auto parent = GetGameObject()->GetParent();

			mWorldMatrix = Matrix4(mTransformMath.GetWorld());

			// If has parent, should apply their world too
			if(parent != nullptr)
				mWorldMatrix = parent->GetTransform()->GetWorld() * mWorldMatrix;

			mWorldDirty = false;
		}

		return mWorldMatrix;
	}

	void TransformComponent::SetLocalWorld(Matrix4 const& mat)
	{
		if (!CanChange())
			return;

		mTransformMath = Transform(mat);
		mWorldDirty = true;
		SetDirty();
		mWorldChanged(this, mTransformMath);
	}

#ifdef _D_EDITOR
	bool TransformComponent::DrawDetails(float params[])
	{
		auto temp = mTransformMath;
		if (D_MATH::DrawDetails(temp, nullptr))
		{
			SetLocalPosition(temp.Translation);
			SetLocalRotation(temp.Rotation);
			SetLocalScale(temp.Scale);
			return true;
		}
		return false;
	}
#endif

}