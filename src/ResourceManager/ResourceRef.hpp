#pragma once

#include "Resource.hpp"

#include <Core/Ref.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

namespace Darius::ResourceManager
{
	template<class T>
	class ResourceRef : public D_CORE::Ref<T>
	{
	public:

		ResourceRef() : ResourceRef(nullptr, std::nullopt) {}

		ResourceRef(D_CORE::CountedOwner ownerData) : ResourceRef(nullptr, ownerData) {}

		ResourceRef(T* data, std::optional<D_CORE::CountedOwner> ownerData = std::nullopt) :
			D_CORE::Ref<T>(data, ownerData)
		{
			using conv = std::is_convertible<T*, Resource*>;
			D_STATIC_ASSERT(conv::value);
		}
	};

	template<class T>
	D_RESOURCE::ResourceRef<T> GetResource(D_CORE::Uuid const& uuid, std::optional<D_CORE::CountedOwner> ownerData = std::nullopt);

}

namespace rttr
{
	template<typename T>
	struct wrapper_mapper<D_RESOURCE::ResourceRef<T>>
	{
		using wrapped_type = D_CORE::Uuid;
		using type = D_RESOURCE::ResourceRef<T>;

		INLINE static wrapped_type get(type const& obj)
		{
			if (!obj.IsValid())
				return D_CORE::Uuid();
			return obj->GetUuid();
		}

		static INLINE type create(wrapped_type const& value)
		{
			return D_RESOURCE::GetResource<T>(value, std::nullopt);
		}

		template<typename U>
		static INLINE D_RESOURCE::ResourceRef<U> convert(type const& source, bool& ok)
		{
			if (auto obj = rttr_cast<U*>(source.Get()))
			{
				ok = true;
				return D_RESOURCE::ResourceRef<U>(source.Get());
			}
			else
			{
				ok = false;
				return D_RESOURCE::ResourceRef<U>(nullptr, std::nullopt);
			}
		}
	};
}

