#pragma once

#include <Core/StringId.hpp>

#ifndef D_UTILS
#define D_UTILS Darius::Utils
#endif

namespace Darius::Utils
{

#ifdef _D_EDITOR

	struct BaseDragDropPayloadContent
	{
		enum class Type
		{
			Invalid = 0,
			Resource,
			GameObject,
			Component
		};

		BaseDragDropPayloadContent(Type type) :
			PayloadType(type)
		{ }

		Type const PayloadType = Type::Invalid;
		virtual bool IsCompatible(Type payloadType, D_CORE::StringId const& type) const = 0;
	};

#endif

}
