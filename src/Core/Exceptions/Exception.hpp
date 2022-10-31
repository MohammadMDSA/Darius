#pragma once

#include <Utils/Common.hpp>

#include <exception>

#ifndef D_EXCEPTION
#define D_EXCEPTION Darius::Core::Exception
#endif // !D_EXCEPTION


namespace Darius::Core::Exception
{
		using Exception = std::exception;

		class NullPointerException : public Exception, public NonCopyable
		{
		public:
			NullPointerException() : Exception("Null pointer exception!") {}
		};

		class FileNotFoundException : public Exception, public NonCopyable
		{
		public:
			FileNotFoundException() : Exception("File not found!") {}
			FileNotFoundException(std::string message) : Exception(message.c_str()) {}
		};

		class UnsupportedException : public Exception, public NonCopyable
		{
		public:
			UnsupportedException() : Exception("Operation is not supported!") {}
			UnsupportedException(std::string message) : Exception(message.c_str()) {}
		};
}