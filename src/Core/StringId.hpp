#pragma once

#include <Libs/StringId/string_id.hpp>
#include "Libs/StringId/database.hpp"


#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	using StringIdHashType = foonathan::string_id::hash_type;

	class StringId : public foonathan::string_id::string_id
	{
	public:
		StringId(foonathan::string_id::string_info str = "");

		StringId(foonathan::string_id::string_info str, foonathan::string_id::basic_database& db);

		StringId(foonathan::string_id::string_info str, foonathan::string_id::basic_database& db, foonathan::string_id::basic_database::insert_status& status);

#if _DEBUG
		std::string RawString;
#endif // _DEBUG

	};
}

namespace std
{
	/// \brief \c std::hash support for \ref string_id.
	template <>
	struct hash<D_CORE::StringId>
	{
		typedef D_CORE::StringId argument_type;
		typedef size_t result_type;

		result_type operator()(const argument_type& arg) const FOONATHAN_NOEXCEPT
		{
			return static_cast<result_type>(arg.hash_code());
		}
	};
} // namspace std

constexpr __forceinline foonathan::string_id::hash_type operator ""_Id(char const* str, std::size_t)
{
	return foonathan::string_id::detail::sid_hash(str);
}

__forceinline D_CORE::StringId operator ""_SId(char const* str, std::size_t)
{
	return D_CORE::StringId(str);
}
