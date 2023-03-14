#pragma once

#include "Serialization.hpp"

#include <Core/Containers/Vector.hpp>

using namespace D_CONTAINERS;
using namespace DirectX;

namespace D_MATH
{
	// Vector 4
	void to_json(D_SERIALIZATION::Json& j, const Vector3& value)
	{
		XMFLOAT3 v = value;
		j = DVector<float>{ v.x, v.y, v.z };
	}

	void from_json(const D_SERIALIZATION::Json& j, Vector3& value)
	{
		value = Vector3(j[0], j[1], j[2]);
	}

	// Vector 4
	void to_json(D_SERIALIZATION::Json& j, const Vector4& value)
	{
		XMFLOAT4 v = value;
		j = DVector<float>{ v.x, v.y, v.z, v.w };
	}

	void from_json(const D_SERIALIZATION::Json& j, Vector4& value)
	{
		value = Vector4(j[0], j[1], j[2], j[3]);
	}

	// Quaternion
	void to_json(D_SERIALIZATION::Json& j, const Quaternion& value)
	{
		j = Vector4(value);
	}

	void from_json(const D_SERIALIZATION::Json& j, Quaternion& value)
	{
		FXMVECTOR values = j.get<Vector4>();
		value = Quaternion(values);
	}

	// Transform
	void to_json(D_SERIALIZATION::Json& j, const Transform& value)
	{
		j["Translation"] = value.Translation;
		j["Rotation"] = value.Rotation;
		j["Scale"] = value.Scale;
	}

	void from_json(const D_SERIALIZATION::Json& j, Transform& value)
	{
		value.Translation = j["Translation"];
		value.Rotation = j["Rotation"];
		value.Scale = j["Scale"];
	}

}