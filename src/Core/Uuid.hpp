#pragma once

#include "Serialization/Json.hpp"

#include <Utils/Common.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	using Uuid = boost::uuids::uuid;
	using UuidHasher = boost::hash<Uuid>;

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

	INLINE std::string ToString(Uuid const& uuid) { return boost::uuids::to_string(uuid); }

	INLINE std::wstring ToWString(Uuid const& uuid) { return boost::uuids::to_wstring(uuid); }

	INLINE Uuid FromString(std::string const& str) { return boost::lexical_cast<boost::uuids::uuid>(str); }

	INLINE Uuid FromWString(std::wstring const& str) { return boost::lexical_cast<boost::uuids::uuid>(str); }

	void to_json(D_SERIALIZATION::Json& j, D_CORE::Uuid const& value);

	void from_json(D_SERIALIZATION::Json const& j, D_CORE::Uuid& value);

	void UuidToJson(D_CORE::Uuid const& value, D_SERIALIZATION::Json& j);

	void UuidFromJson(D_CORE::Uuid& value, D_SERIALIZATION::Json const& j);

}

namespace std
{
	template<>
	struct hash<D_CORE::Uuid>
	{
		INLINE size_t operator() (D_CORE::Uuid const& uuid) const
		{
			boost::hash<D_CORE::Uuid> hasher;
			return hasher(uuid);
		}
	};
}
