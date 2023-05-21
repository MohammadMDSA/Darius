#pragma once

#include "Json.hpp"

#include "Core/Containers/Map.hpp"
#include "Core/Uuid.hpp"

#include <rttr/type.h>

#include <functional>

#ifndef D_SERIALIZATION
#define D_SERIALIZATION Darius::Core::Serialization
#endif // !D_SERIALIZATION

namespace Darius::Core::Serialization
{

	struct SerializationContext
	{
		bool							Rereference = false;
		bool							MaintainExternalReferences = true;
		D_CONTAINERS::DUnorderedMap<D_CORE::Uuid, D_CORE::Uuid, D_CORE::UuidHasher>	ReferenceMap;
	};

	void Serialize(rttr::instance const obj, Json& json, SerializationContext const& context);
	void Serialize(rttr::instance const obj, Json& json);
	void SerializeSequentialContainer(rttr::variant const& var, Json& json);
	void Deserialize(rttr::instance obj, Json const& json);

	// Don't call unless you know what you're exacly doing
	bool __RegisterSerializer(rttr::type type, std::function<void(rttr::instance const&, Json&)> serializer, std::function<void(rttr::variant&, Json const&)> deserializer);

	template<typename T>
	bool RegisterSerializer(std::function<void(T const&, Json&)> serializer, std::function<void(T&, Json const&)> deserializer)
	{
		std::function<void(rttr::instance const&, Json&)> serFunc = nullptr;
		if (serializer)
			serFunc = [serializer](rttr::instance const& var, Json& j)
		{
			T* val = var.try_convert<T>();
			serializer(*val, j);
		};

		std::function<void(rttr::variant&, Json const&)> deserFunc = nullptr;
		if (deserializer)
			deserFunc = [deserializer](rttr::variant& var, Json const& j)
		{
			T value;
			deserializer(value, j);
			var = value;
		};

		return __RegisterSerializer(
			rttr::type::get<T>(),
			serFunc,
			deserFunc
		);
	}

}