#pragma once

#include <Libs/StringId/string_id.hpp>
#include "Libs/StringId/database.hpp"


#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	using StringId = foonathan::string_id::string_id;
	using StringIdDatabase = foonathan::string_id::default_database;
	using StringIdHashType = foonathan::string_id::hash_type;

	extern StringIdDatabase DefaultStringIdDatabase;
}

constexpr __forceinline foonathan::string_id::hash_type operator ""_Id(char const* str, std::size_t)
{
	return foonathan::string_id::detail::sid_hash(str);
}

__forceinline D_CORE::StringId operator ""_SId(char const* str, std::size_t)
{
	return D_CORE::StringId(str, D_CORE::DefaultStringIdDatabase);
}
