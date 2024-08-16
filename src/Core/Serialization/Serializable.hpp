#pragma once

#include "Json.hpp"

#include <rttr/rttr_enable.h>

#ifndef D_SERIALIZATION
#define D_SERIALIZATION Darius::Core::Serialization
#endif // !D_SERIALIZATION

namespace Darius::Core::Serialization
{
	class ISerializable
	{
		RTTR_ENABLE();
	public:
		virtual void Serialize(Json& json) const = 0;
		virtual void Deserialize(Json const& json) = 0;
	};
}
