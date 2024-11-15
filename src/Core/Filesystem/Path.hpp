#pragma once

#include <filesystem>

#ifndef D_FILE
#define D_FILE Darius::Core::Filesystem
#endif // !D_FILE

namespace Darius::Core::Filesystem
{
	typedef std::filesystem::path Path;
}