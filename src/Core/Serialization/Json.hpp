#pragma once

#include <nlohmann/json.hpp>

#ifndef D_SERIALIZATION
#define D_SERIALIZATION Darius::Core::Serialization
#endif // !D_SERIALIZATION

#define D_H_SERIALIZE_ENUM(type, ...) NLOHMANN_JSON_SERIALIZE_ENUM(type, __VA_ARGS__)

#define D_H_SERIALIZE_VALUE(name) j[D_NAMEOF(name)] = value.m##name
#define D_H_DESERIALIZE_VALUE(name) value.m##name = j[D_NAMEOF(name)]
#define D_H_SERIALIZE(prop) json[D_NAMEOF(prop)] = m##prop;
#define D_H_DESERIALIZE(prop) \
{ \
    if(json.contains(D_NAMEOF(prop))) \
        m##prop = json[D_NAMEOF(prop)]; \
}

#define D_H_DESERIALIZE_REF_PROP(prop) \
{ \
    if(json.contains(D_NAMEOF(prop))) \
    { \
        auto uuid = D_CORE::FromString(json[D_NAMEOF(prop)]); \
        m##prop##Handle = D_RESOURCE::GetResourceHandle(uuid); \
    } \
}

#define D_H_DESERIALIZE_SETTER(prop, setter) \
{ \
    if(json.contains(#prop)) \
        setter(json[#prop]); \
}

namespace Darius::Core::Serialization
{
	using Json = nlohmann::json;
}