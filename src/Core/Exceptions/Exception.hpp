#pragma once

#include <Utils/Common.hpp>

#include <exception>

#ifndef D_EXCEPTION
#define D_EXCEPTION Darius::Core::Exception
#endif // !D_EXCEPTION


namespace Darius::Core::Exception
{
		using Exception = std::exception;

		class NullPointerException : NonCopyable, public Exception
		{
		public:
			NullPointerException() : Exception("Null pointer exception!") {}
		};

		class FileNotFoundException : NonCopyable, public Exception
		{
		public:
			FileNotFoundException() : Exception("File not found!") {}
			FileNotFoundException(std::string message) : Exception(message.c_str()) {}
		};
}