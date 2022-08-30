#pragma once

#include <Core/Serialization/Json.hpp>
#include <Math/VectorMath.hpp>
#include <Renderer/LightManager.hpp>

namespace D_MATH
{
	// Vector 3
	void to_json(D_SERIALIZATION::Json& j, const Vector3& value);
	void from_json(const D_SERIALIZATION::Json& j, Vector3& value);

	// Vector 4
	void to_json(D_SERIALIZATION::Json& j, const Vector4& value);
	void from_json(const D_SERIALIZATION::Json& j, Vector4& value);

	// Quaternion
	void to_json(D_SERIALIZATION::Json& j, const Quaternion& value);
	void from_json(const D_SERIALIZATION::Json& j, Quaternion& value);

	// Transform
	void to_json(D_SERIALIZATION::Json& j, const Transform& value);
	void from_json(const D_SERIALIZATION::Json& j, Transform& value);
}

namespace D_LIGHT
{
	D_H_SERIALIZE_ENUM(LightSourceType,
		{
			{ LightSourceType::DirectionalLight, "DirectionalLight" },
			{ LightSourceType::PointLight, "PointLight" },
			{ LightSourceType::SpotLight, "SpotLight" }
		});
}