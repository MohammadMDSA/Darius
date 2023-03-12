#include "Scene/pch.hpp"
#include "TransformComponent.hpp"

#include "Scene/Utils/DetailsDrawer.hpp"
#include "Scene/Utils/Serializer.hpp"

using namespace D_SERIALIZATION;

namespace Darius::Math
{
	D_H_COMP_DEF(TransformComponent);

	TransformComponent::TransformComponent() : D_ECS_COMP::ComponentBase() {}

	TransformComponent::TransformComponent(D_CORE::Uuid uuid) : D_ECS_COMP::ComponentBase(uuid) {}

#ifdef _D_EDITOR
	bool TransformComponent::DrawDetails(float params[])
	{
		auto temp = mTransform;
		if (D_MATH::DrawDetails(temp, nullptr))
		{
			SetLocalTransform(temp);
			return true;
		}
		return false;
	}
#endif

	void TransformComponent::Serialize(Json& j) const
	{
		j = mTransform;
	}

	void TransformComponent::Deserialize(Json const& j)
	{
		mTransform = j;
	}
}