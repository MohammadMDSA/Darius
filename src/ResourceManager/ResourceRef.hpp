#pragma once

#include "Resource.hpp"
#include "ResourceLoader.hpp"

#include <Core/RefCounting/Ref.hpp>

#ifndef D_RESOURCE
#define D_RESOURCE Darius::ResourceManager
#endif // !D_RESOURCE

namespace Darius::ResourceManager
{
	template<class T>
	class ResourceRef : public D_CORE::Ref<T>
	{
	public:

		ResourceRef(ResourceRef const& other) : D_CORE::Ref<T>(other) { }

		template<class OTHER>
		ResourceRef(D_CORE::Ref<OTHER> const& other) : D_CORE::Ref<T>(other) { };

		ResourceRef(T* ptr) : D_CORE::Ref<T>(ptr) { }

		ResourceRef() : D_CORE::Ref<T>() { };

		ResourceRef& operator= (ResourceRef const&) = default;

		INLINE bool IsValidAndGpuDirty() const { return D_CORE::Ref<T>::IsValid() && D_CORE::Ref<T>::Get()->IsDirtyGPU(); }
	};

	template<class T>
	D_RESOURCE::ResourceRef<T> GetResourceSync(D_CORE::Uuid const& uuid);
	Resource* GetRawResourceSync(D_CORE::Uuid const& uuid, bool syncLoad);

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
			if (value.is_nil())
				return D_RESOURCE::ResourceRef<T>(nullptr);
			auto res = D_RESOURCE::GetRawResourceSync(value, false);
			D_RESOURCE_LOADER::LoadResourceAsync(res, nullptr, true);
			T* ref = dynamic_cast<T*>(res);
			D_ASSERT(ref);
			return ref;
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

