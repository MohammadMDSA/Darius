#include "Core/pch.hpp"
#include "Uuid.hpp"

using namespace D_SERIALIZATION;

namespace Darius::Core
{
	void to_json(Json& j, const D_CORE::Uuid& value) {
		j = ToString(value);
	}

	void from_json(const Json& j, D_CORE::Uuid& value) {
		value = FromString(j);
	}

	void UuidToJson(const D_CORE::Uuid& value, Json& j) {
		j = ToString(value);
	}

	void UuidFromJson(D_CORE::Uuid& value, const Json& j) {
		value = FromString(j);
	}
}