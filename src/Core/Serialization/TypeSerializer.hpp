#pragma once

#include "Json.hpp"

#include <Core/Containers/Map.hpp>

#include <rttr/type.h>

#include <functional>

#ifndef D_SERIALIZATION
#define D_SERIALIZATION Darius::Core::Serialization
#endif // !D_SERIALIZATION

namespace Darius::Core::Serialization
{
	void Serialize(rttr::instance const& obj, Json& json);
	void Deserialize(rttr::instance obj, Json const& json);

	template<typename T>
	bool RegisterSerializer(std::function<void(T const&, Json&)> serializer, std::function<void(T&, Json const&)> deserializer)
	{
		return __RegisterSerializer(
			rttr::type::get<T>(),
			[serializer](rttr::instance const& ins, Json& j)
				{
					T* var = ins.try_convert<T>();
					serializer(*var, j);
				},
			[deserializer](rttr::instance& ins, Json const& j)
				{
					T* obj = ins.try_convert<T>();
					deserializer(*obj, j);
				}
		);
	}

	// Don't call unless you know what you're exacly doing
	bool __RegisterSerializer(rttr::type type, std::function<void(rttr::instance const&, Json&)> serializer, std::function<void(rttr::instance&, Json const&)> deserializer);
}