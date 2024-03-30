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
	// Provides a context for serialization to know the strategy of changing references of nested entities
	struct SerializationContext
	{
		// Maintain references between nested entities
		bool							Rereference = false;

		// Maintain references to external entities
		bool							MaintainExternalReferences = true;

		// Reference map to lookup from
		D_CONTAINERS::DUnorderedMap<D_CORE::Uuid, D_CORE::Uuid, D_CORE::UuidHasher> const&	ReferenceMap;
	};

#define D_SERIALIZATION_UUID_PARAM_GAMEOBJECT 423843029
	struct UuidWrapper
	{
		Uuid							Uuid;
		int								Param;
	};

	void Initialize();
	void Shutdown();

	void Serialize(rttr::instance const obj, Json& json, SerializationContext const& context);
	void Serialize(rttr::instance const obj, Json& json);
	void SerializeSequentialContainer(rttr::variant const& var, Json& json, SerializationContext const& context);
	void SerializeSequentialContainer(rttr::variant const& var, Json& json);
	void Deserialize(rttr::instance obj, Json const& json);

	INLINE void DeserializeUuidWrapper(UuidWrapper& wrapper, Json const& json)
	{
		UuidFromJson(wrapper.Uuid, json);
	}

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