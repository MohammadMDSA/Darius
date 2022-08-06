#pragma once

#include <Utils/Common.hpp>

#include <exception>

#ifndef D_EXCEPTION
#define D_EXCEPTION Darius::Core::Exception
#endif // !D_EXCEPTION


namespace Darius::Core::Exception
{
		using Exception = std::exception;

		class NullPointerException : NonCopyable, public std::exception
		{
		public:
			NullPointerException() : std::exception("Nullpointer Exception") {}
		};
}