#include "pch.hpp"

#include "StringId/string_id.hpp"
#include "StringId/database.hpp"


#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	using StringId = foonathan::string_id::string_id;
	using StringIdDatabase = foonathan::string_id::default_database;
}