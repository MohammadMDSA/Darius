#pragma once

#include <nlohmann/json.hpp>

#ifndef D_SERIALIZATION
#define D_SERIALIZATION Darius::Core::Serialization
#endif // !D_SERIALIZATION

#define D_H_SERIALIZE_ENUM(type, ...) NLOHMANN_JSON_SERIALIZE_ENUM(type, __VA_ARGS__)

#define D_H_SERIALIZE(name) j[D_NAMEOF(name)] = value.m##name
#define D_H_DESERIALIZE(name) value.m##name = j[D_NAMEOF(name)]

namespace Darius::Core::Serialization
{
	using Json = nlohmann::json;
}