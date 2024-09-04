#include "pch.hpp"

#include "StringId.hpp"

using namespace foonathan::string_id;

namespace Darius::Core
{
	std::unique_ptr<default_database> DefaultStringIdDatabase = std::make_unique<default_database>();

	StringId::StringId(string_info str) : 
		string_id(str, *DefaultStringIdDatabase)
#if _DEBUG
		, RawString(str.string)
#endif // _DEBUG
	{}


	StringId::StringId(string_info str, basic_database& db) :
		string_id(str, db)
#if _DEBUG
		, RawString(str.string)
#endif // _DEBUG
	{}

	StringId::StringId(string_info str, basic_database& db, basic_database::insert_status& status) :
		string_id(str, db, status)
#if _DEBUG
		, RawString(str.string)
#endif // _DEBUG
	{}

}

