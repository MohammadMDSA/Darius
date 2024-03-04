#pragma once

#include "Json.hpp"

#include <rttr/rttr_enable.h>

#ifndef D_SERIALIZATION
#define D_SERIALIZATION Darius::Core::Serialization
#endif // !D_SERIALIZATION

namespace Darius::Core::Serialization
{
	class ICopyable
	{
		RTTR_ENABLE();
	public:

		virtual void Copy(bool maintainContext, Json& serialized) const = 0;

		virtual bool IsCopyableValid() const = 0;
	};

}
