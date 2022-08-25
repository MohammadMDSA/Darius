#include "Core/pch.hpp"
#include "Uuid.hpp"

using namespace D_SERIALIZATION;

namespace Darius::Core
{
	void to_json(Json& j, const D_CORE::Uuid& value) {
		j = value.data;
	}

	void from_json(const Json& j, D_CORE::Uuid& value) {
		for (size_t i = 0; i < 16; i++)
		{
			value.data[i] = j[i];
		};
	}
}