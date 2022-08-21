#pragma once

#include <Utils/Common.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <string>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	using Uuid = boost::uuids::uuid;

	INLINE Uuid GenerateUuid()
	{
		boost::uuids::random_generator generator;
		return generator();
	}

	INLINE Uuid GenerateUuidFor(std::string name)
	{
		boost::uuids::name_generator generator(boost::uuids::ns::oid());
		return generator(name);
	}

	INLINE Uuid GenerateUuidFor(std::wstring name)
	{
		boost::uuids::name_generator generator(boost::uuids::ns::oid());
		return generator(name);
	}

	INLINE std::string ToString(Uuid uuid) { return boost::uuids::to_string(uuid); }

	INLINE std::wstring ToWString(Uuid uuid) { return boost::uuids::to_wstring(uuid); }
}