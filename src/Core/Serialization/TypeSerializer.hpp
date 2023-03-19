#pragma once

#include "Json.hpp"

#include <rttr/type.h>

#ifndef D_SERIALIZATION
#define D_SERIALIZATION Darius::Core::Serialization
#endif // !D_SERIALIZATION

namespace Darius::Core::Serialization
{
	void Serialize(rttr::instance const& obj, Json& json);
	void Deserialize(rttr::instance obj, Json const& json);
}